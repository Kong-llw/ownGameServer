#pragma once

#include "core/Types.h"
#include <optional>
#include <string>
struct UserBaseInfo;
//修改BaseInfo的接口
class IUserStateStore {
public:
    virtual ~IUserStateStore() = default;

    virtual std::optional<UserBaseInfo> GetUserState(UserId id) const = 0;
    virtual bool UpdateUserGroup(UserId id, GroupId new_group_id) = 0;
    virtual bool UpdateUserOnlineStatus(UserId id, bool is_online) = 0;
    virtual bool UpdateUserName(UserId id, const std::string& new_name) = 0;
    virtual bool UpdateUserSession(UserId id, SessionId new_session_id) = 0;
    virtual bool UpdateUserState(const UserBaseInfo& info) = 0;
};
