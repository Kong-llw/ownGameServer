#include "network/session/SessionManager.hpp"

#include <mutex>
#include <utility>
#include <iostream>

#include "network/session/TcpConnection.hpp"
#include "protocol/Router/UserSessionMap.hpp"

namespace Network {

SessionManager::SessionManager(asio::any_io_executor exec,
     std::shared_ptr<UserSessionMap> user_session_map,
     std::shared_ptr<IBusinessMsgGateway> gateway)
    : executor_(std::move(exec)),
      //user_session_map_(std::move(user_session_map)),
      gateway_(std::move(gateway)) {}

void SessionManager::SetOnSessionClose(SessionCloseHandler cb) {
    std::unique_lock lock(mutex_);
    on_session_close_ = std::move(cb);
}

SessionId SessionManager::AllocateSessionId() {
    return next_session_id_.fetch_add(1, std::memory_order_relaxed);
}

SessionId SessionManager::CreateSession(tcp::socket socket, std::shared_ptr<IMessageCodec> codec) {
    if (!gateway_ || !codec || !socket.is_open()) {
        return SessionId{};
    }
    const SessionId session_id = AllocateSessionId();
    if (session_id == SessionId{}) {
        return SessionId{};
    }

    std::shared_ptr<ClientSession> session;
    std::shared_ptr<TcpConnection> connection;
    bool session_inserted = false;

    try {
        session = std::make_shared<ClientSession>(session_id, std::move(codec), gateway_);
        connection = std::make_shared<TcpConnection>(std::move(socket), session_id);
        session->SetConnection(connection);

        //给TcpConnect注入一个 对 ClientSession的反向路径 用于传回socket读取的内容
        connection->SetMessageCallback([weak_session = std::weak_ptr<ClientSession>(session)](std::span<std::byte> payload) {
            if (auto strong_session = weak_session.lock()) {
                strong_session->onSocketRecv(payload);
            }
        });

        //TcpConnect关闭后，触发closeSession 回收ClientSession
        auto weak_mgr = weak_from_this();
        connection->SetCloseCallback([weak_mgr, session_id]() {
            if (auto mgr = weak_mgr.lock()) {
                mgr->CloseSession(session_id);
            }
        });

        //插入
        {
            std::unique_lock lock(mutex_);
            if (sessions_.size() >= kMaxConnectionNum) {
                //LOG_ERROR("CreateSession failed: reach max connection num ({})", kMaxConnectionNum);
                return SessionId{};
            }
            
            auto [it, inserted] = sessions_.emplace(session_id, std::move(session));
            if (!inserted) {
                return SessionId{};
            }
        }
        session_inserted = true;
        connection->Start();

        return session_id;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        if (session_inserted) {
            std::unique_lock lock(mutex_);
            sessions_.erase(session_id);
        }
        return SessionId{};
    }
}

bool SessionManager::CloseSession(SessionId session_id) {
    std::shared_ptr<ClientSession> removed;
    SessionCloseHandler callback;

    {
        std::unique_lock lock(mutex_);
        auto it = sessions_.find(session_id);
        if (it == sessions_.end()) {
            return false;
        }
        removed = std::move(it->second);
        sessions_.erase(it);
        callback = on_session_close_;
    }

    if (callback && removed) {
        callback(std::move(removed));
    }
    return true;
}

bool SessionManager::IsConnectionFull() const {
    std::shared_lock lock(mutex_);
    return sessions_.size() >= kMaxConnectionNum;
}

std::shared_ptr<ClientSession> SessionManager::GetSession(SessionId session_id) const {
    std::shared_lock lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace Network
