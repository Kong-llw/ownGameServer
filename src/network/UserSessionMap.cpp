#include "network/UserSessionMap.hpp"

#include <mutex>

namespace Network {

void UserSessionMap::Bind(UserId user_id, std::shared_ptr<ClientSession> session) {
    std::unique_lock lock(mutex_);
    user_to_session_[user_id] = std::move(session);
}

bool UserSessionMap::Unbind(UserId user_id) {
    std::unique_lock lock(mutex_);
    return user_to_session_.erase(user_id) > 0;
}

bool UserSessionMap::UnbindBySessionId(SessionId session_id) {
    std::unique_lock lock(mutex_);
    for (auto it = user_to_session_.begin(); it != user_to_session_.end(); ++it) {
        if (it->second && it->second->GetSessionId() == session_id) {
            user_to_session_.erase(it);
            return true;
        }
    }
    return false;
}

std::shared_ptr<ClientSession> UserSessionMap::GetSession(UserId user_id) const {
    std::shared_lock lock(mutex_);
    auto it = user_to_session_.find(user_id);
    if (it == user_to_session_.end()) {
        return nullptr;
    }
    return it->second;
}

SessionId UserSessionMap::GetSessionId(UserId user_id) const {
    auto session = GetSession(user_id);
    return session ? session->GetSessionId() : SessionId{};
}

std::size_t UserSessionMap::Size() const {
    std::shared_lock lock(mutex_);
    return user_to_session_.size();
}

} // namespace Network
