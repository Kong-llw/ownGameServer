#include "network/session/SessionManager.hpp"

#include <mutex>

#include "protocol/Router/UserSessionMap.hpp"

namespace Network {

SessionManager::SessionManager(asio::any_io_executor exec, std::shared_ptr<UserSessionMap> user_session_map)
    : executor_(std::move(exec)),
      user_session_map_(std::move(user_session_map)) {}

void SessionManager::SetOnSessionClose(SessionCloseHandler cb) {
    std::unique_lock lock(mutex_);
    on_session_close_ = std::move(cb);
}

SessionId SessionManager::AllocateSessionId() {
    return next_session_id_.fetch_add(1, std::memory_order_relaxed);
}

SessionId SessionManager::CreateSession(std::shared_ptr<ClientSession> session) {
    if (!session) {
        return SessionId{};
    }

    const SessionId sid = session->GetSessionId();
    if (sid == SessionId{}) {
        return SessionId{};
    }

    std::unique_lock lock(mutex_);
    if (sessions_.size() >= kMaxConnectionNum) {
        return SessionId{};
    }

    auto [it, inserted] = sessions_.emplace(sid, std::move(session));
    if (!inserted) {
        return SessionId{};
    }
    return it->first;
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

    if (user_session_map_) {
        user_session_map_->UnbindBySessionId(session_id);
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
