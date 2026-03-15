#include "UserSessionMap.hpp"

#include <mutex>

namespace Network {

void UserSessionMap::Bind(UserId user_id, SessionId session_id) {
    std::unique_lock lock(mutex_);
    auto old = user_to_session_.find(user_id);
    if (old != user_to_session_.end()) {
        session_to_user_.erase(old->second);
    }
    user_to_session_[user_id] = session_id;
    session_to_user_[session_id] = user_id;
}

bool UserSessionMap::Unbind(UserId user_id) {
    std::unique_lock lock(mutex_);
    auto it = user_to_session_.find(user_id);
    if (it == user_to_session_.end()) {
        return false;
    }
    session_to_user_.erase(it->second);
    user_to_session_.erase(it);
    return true;
}

bool UserSessionMap::UnbindBySessionId(SessionId session_id) {
    std::unique_lock lock(mutex_);
    auto it = session_to_user_.find(session_id);
    if (it == session_to_user_.end()) {
        return false;
    }
    const UserId uid = it->second;
    session_to_user_.erase(it);
    user_to_session_.erase(uid);
    return true;
}

SessionId UserSessionMap::GetSessionId(UserId user_id) const {
    std::shared_lock lock(mutex_);
    auto it = user_to_session_.find(user_id);
    if (it == user_to_session_.end()) {
        return SessionId{};
    }
    return it->second;
}

UserId UserSessionMap::GetUserId(SessionId session_id) const {
    std::shared_lock lock(mutex_);
    auto it = session_to_user_.find(session_id);
    if (it == session_to_user_.end()) {
        return UserId{};
    }
    return it->second;
}

std::size_t UserSessionMap::Size() const {
    std::shared_lock lock(mutex_);
    return user_to_session_.size();
}

void UserSessionMap::onUserLogin(UserLoginInfo info) {
    if (info.result != MsgProto::LoginResult::SUCCESS || info.user_id == UserId{} || info.session_id == SessionId{}) {
        return;
    }
    Bind(info.user_id, info.session_id);
}

void UserSessionMap::onUserLogout(UserLoginInfo info) {
    if (info.user_id != UserId{}) {
        Unbind(info.user_id);
        return;
    }
    if (info.session_id != SessionId{}) {
        UnbindBySessionId(info.session_id);
    }
}

} // namespace Network
