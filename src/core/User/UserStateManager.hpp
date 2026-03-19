#pragma once
//此文件弃用 功能分给 PlayerManger 和Player
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <mutex>

#include "UserBaseInfo.hpp"
#include "IUserStateStore.hpp"

struct UserLoginInfo;

class UserStateManager : public IUserStateStore {
private:
    std::unordered_map<UserId, UserBaseInfo> user_states; //未登录的不参与业务
    mutable std::shared_mutex state_mutex;

public:
    ~UserStateManager() = default;
    void OnUserLogin(const UserLoginInfo& info);
    void OnUserLogout(const UserLoginInfo& info);
    std::optional<UserBaseInfo> GetUserState(UserId id) const override;
    bool UpdateUserGroup(UserId id, GroupId new_group_id) override;
    bool UpdateUserOnlineStatus(UserId id, bool is_online) override;
    bool UpdateUserName(UserId id, const std::string& new_name) override;
    bool UpdateUserSession(UserId id, SessionId new_session_id) override;
    bool UpdateUserState(const UserBaseInfo& info) override;
};  