#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "core/Types.h"
#include "network/session/ClientSession.hpp"
//只是起到一个转接表的功能，将用户ID和网络连接（Session）关联起来，供业务层查询
namespace Network {

class UserSessionMap {
public:
    void Bind(UserId user_id, std::shared_ptr<ClientSession> session);
    bool Unbind(UserId user_id);
    bool UnbindBySessionId(SessionId session_id);

    std::shared_ptr<ClientSession> GetSession(UserId user_id) const;
    SessionId GetSessionId(UserId user_id) const;

    std::size_t Size() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<UserId, std::shared_ptr<ClientSession>> user_to_session_;
};

} // namespace Network
