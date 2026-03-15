#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "core/Types.h"
#include "protocol/MessageProto.hpp"
//只是起到一个转接表的功能，将用户ID和网络连接（Session）关联起来，供业务层查询
//逻辑上视为Router的一部分
namespace Network {

class MsgRouter;

class UserSessionMap {
public:
    SessionId GetSessionId(UserId user_id) const;
    UserId GetUserId(SessionId session_id) const;
    std::size_t Size() const;
    void onUserLogin(UserLoginInfo info);
    void onUserLogout(UserLoginInfo info);

private:
    friend class MsgRouter;

    mutable std::shared_mutex mutex_;
    void Bind(UserId user_id, SessionId session_id);
    bool Unbind(UserId user_id);
    bool UnbindBySessionId(SessionId session_id);
    std::unordered_map<UserId, SessionId> user_to_session_;
    std::unordered_map<SessionId, UserId> session_to_user_;
};

} // namespace Network
