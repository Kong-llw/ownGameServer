#include "network/session/SessionManager.hpp"

#include <chrono>
#include <mutex>
#include <utility>
#include <iostream>

#include "network/session/TcpConnection.hpp"
#include "protocol/Router/UserSessionMap.hpp"

namespace Network {

SessionManager::SessionManager(asio::any_io_executor exec,
     std::shared_ptr<IBusinessMsgGateway> gateway)
    : executor_(std::move(exec)),
    heartbeat_sweep_timer_(executor_),
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
            last_heartbeat_at_[session_id] = SteadyClock::now();
        }
        session_inserted = true;
        connection->Start();
        //调试过程先关闭心跳
        //EnsureHeartbeatSweepStarted();

        return session_id;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        if (session_inserted) {
            std::unique_lock lock(mutex_);
            sessions_.erase(session_id);
            last_heartbeat_at_.erase(session_id);
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
        last_heartbeat_at_.erase(session_id);
        callback = on_session_close_;
    }

    if (removed) {
        removed->Close();
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

void SessionManager::onSessionHeartbeat(SessionId session_id) {
    std::unique_lock lock(mutex_);
    if (sessions_.find(session_id) == sessions_.end()) {
        return;
    }
    last_heartbeat_at_[session_id] = SteadyClock::now();
}

void SessionManager::EnsureHeartbeatSweepStarted() {
    bool should_start = false;
    {
        std::unique_lock lock(mutex_);
        if (!heartbeat_sweep_running_) {
            heartbeat_sweep_running_ = true;
            should_start = true;
        }
    }

    if (should_start) {
        ScheduleHeartbeatSweep();
    }
}

void SessionManager::ScheduleHeartbeatSweep() {
    heartbeat_sweep_timer_.expires_after(kHeartbeatCheckInterval);
    heartbeat_sweep_timer_.async_wait([weak_self = weak_from_this()](const std::error_code& ec) {
        if (auto self = weak_self.lock()) {
            self->HandleHeartbeatSweep(ec);
        }
    });
}

void SessionManager::HandleHeartbeatSweep(const std::error_code& ec) {
    if (ec == asio::error::operation_aborted) {
        return;
    }

    const auto now = SteadyClock::now();
    std::vector<SessionId> expired_sessions;

    {
        std::unique_lock lock(mutex_);
        for (const auto& [sid, heartbeat_at] : last_heartbeat_at_) {
            if (sessions_.find(sid) == sessions_.end()) {
                continue;
            }
            if (now - heartbeat_at > kHeartbeatTimeout) {
                expired_sessions.push_back(sid);
            }
        }
    }

    for (const SessionId sid : expired_sessions) {
        CloseSession(sid);
    }

    {
        std::unique_lock lock(mutex_);
        if (sessions_.empty()) {
            heartbeat_sweep_running_ = false;
            return;
        }
    }

    ScheduleHeartbeatSweep();
}
} // namespace Network
