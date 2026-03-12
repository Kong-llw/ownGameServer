#include "MsgRouter.hpp"

#include <stdexcept>

namespace Network { 

MsgRouter::MsgRouter(std::shared_ptr<UserSessionMap> user_session_map)
    : user_session_map_(std::move(user_session_map)) {
    if (!user_session_map_) {
        throw std::invalid_argument("MsgRouter requires a non-null UserSessionMap");
    }
}

bool MsgRouter::RegisterUserSession(UserId user_id, std::shared_ptr<ClientSession> session) {
    user_session_map_->Bind(user_id, std::move(session));
    return true;
}

bool MsgRouter::UnregisterUserSession(UserId user_id) {
    return user_session_map_->Unbind(user_id);
}

bool MsgRouter::UnregisterSessionById(SessionId session_id) {
    return user_session_map_->UnbindBySessionId(session_id);
}

bool MsgRouter::SendMessageToUser(UserId id, std::span<const std::byte> msg) {
    auto session = user_session_map_->GetSession(id);
    if (!session) {
        return false;
    }
    return session->SendMessage(msg);
}

bool MsgRouter::BroadcastToRoom(RoomId id, std::span<const std::byte> msg) {
    (void)id;
    (void)msg;
    // Implementation here
    return false;
}

bool MsgRouter::MsgDispatchToLogic(const std::vector<std::byte>& msg, ProtoInfo::ProtocolType type) {
    (void)msg;
    (void)type;
    // Implementation here
    return false;
}

}