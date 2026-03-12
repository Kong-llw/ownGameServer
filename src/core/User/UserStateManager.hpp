#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <mutex>

#include "UserBaseInfo.hpp"
#include "IUserStateStore.hpp"

class UserStateManager : public IUserStateStore {
private:
    std::unordered_map<SessionId, UserBaseInfo> user_states;
    std::shared_mutex state_mutex; 

public:
    ~UserStateManager() = default;
    std::optional<UserBaseInfo> GetUserState(UserId id) const override;
    bool UpdateUserGroup(UserId id, GroupId new_group_id) override;
    bool UpdateUserOnlineStatus(UserId id, bool is_online) override;
    bool UpdateUserName(UserId id, const std::string& new_name) override;
    bool UpdateUserSession(UserId id, SessionId new_session_id) override;
    bool UpdateUserState(const UserBaseInfo& info) override;
};  