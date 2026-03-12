#pragma once
#include "IBusinessMsgGateway.hpp"
#include "network/session/IMessageSender.hpp"
#include "network/UserSessionMap.hpp"

namespace Network {
class MsgRouter : public IBusinessMsgGateway {
public:
    explicit MsgRouter(UserSessionMap& connection_manager) : connection_manager_(connection_manager) {}
    ~MsgRouter() override = default;

    template <ContiguousContainer C>
    bool SendToUser(UserId user_id, const C& payload) const {
        auto bytes = std::as_bytes(std::span(payload));
        return SendMessageToUser(user_id, bytes);
    }

    bool SendMessageToUser(UserId id, std::span<const std::byte> msg) override;
    bool BroadcastToRoom(RoomId id, std::span<const std::byte> msg) override;
private:
    UserSessionMap& connection_manager_;
};  
}
