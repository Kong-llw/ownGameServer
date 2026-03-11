#include "MsgRouter.hpp"

namespace Network { 

bool MsgRouter::SendMessageToUser(UserId id, std::span<const std::byte> msg) {
    auto session = connection_manager_.GetSession(id);
    if (!session) {
        return false;
    }
    return session->SendMessage(msg);
}

bool MsgRouter::BroadcastToRoom(RoomId id, std::span<const std::byte> msg) {
    // Implementation here
    return false;
}
}