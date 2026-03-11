#pragma once
#include "IBusinessMsgGateway.hpp"
#include "network/session/IMessageSender.hpp"
#include "network/UserConnectionManager.hpp"

namespace Network {
class MsgRouter : public IBusinessMsgGateway {
public:
    explicit MsgRouter(UserConnectionManager& connection_manager) : connection_manager_(connection_manager) {}
    ~MsgRouter() override = default;

    template <ContiguousContainer C>
    bool SendToUser(UserId user_id, const C& payload) const {
        auto bytes = std::as_bytes(std::span(payload));
        return SendMessageToUser(user_id, bytes);
    }

    bool SendMessageToUser(UserId id, std::span<const std::byte> msg) override;
    bool BroadcastToRoom(RoomId id, std::span<const std::byte> msg) override;
private:
    UserConnectionManager& connection_manager_;
};  
}
