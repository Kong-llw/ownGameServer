#include "UserStateManager.hpp"
#include <optional>
#include <string>
#include "protocol/MessageProto.hpp"

void UserStateManager::OnUserLogin(const UserLoginInfo& info) {
    if (info.result != MsgProto::LoginResult::SUCCESS || info.user_id == UserId{}) {
        return;
    }

    UserBaseInfo state{
        info.user_id,
        0,
        "player_" + std::to_string(info.user_id),
        info.session_id,
        true,
    };
    UpdateUserState(state);
}

void UserStateManager::OnUserLogout(const UserLoginInfo& info) {
    if (info.user_id == UserId{}) {
        return;
    }
    UpdateUserOnlineStatus(info.user_id, false);
}

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