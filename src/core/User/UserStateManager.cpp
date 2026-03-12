#include "UserStateManager.hpp"
#include <optional>
#include <string>

std::optional<UserBaseInfo> UserStateManager::GetUserState(UserId id) const {
    std::shared_lock lock(state_mutex);
    auto it = user_states.find(id);
    if (it == user_states.end()) return std::nullopt;
    return it->second;
}

bool UserStateManager::UpdateUserGroup(UserId id, GroupId new_group_id) {
    std::unique_lock lock(state_mutex);
    auto it = user_states.find(id);
    if (it == user_states.end()) return false;
    it->second.current_group_id = new_group_id;
    return true;
}

bool UserStateManager::UpdateUserOnlineStatus(UserId id, bool is_online) {
    std::unique_lock lock(state_mutex);
    auto it = user_states.find(id);
    if (it == user_states.end()) return false;
    it->second.is_online = is_online;
    return true;
}

bool UserStateManager::UpdateUserName(UserId id, const std::string& new_name) {
    std::unique_lock lock(state_mutex);
    auto it = user_states.find(id);
    if (it == user_states.end()) return false;
    it->second.user_name = new_name;
    return true;
}

bool UserStateManager::UpdateUserSession(UserId id, SessionId new_session_id) {
    std::unique_lock lock(state_mutex);
    auto it = user_states.find(id);
    if (it == user_states.end()) return false;
    it->second.session_id = new_session_id;
    return true;
}

bool UserStateManager::UpdateUserState(const UserBaseInfo& info) {
    std::unique_lock lock(state_mutex);
    user_states[info.user_id] = info;
    return true;
}