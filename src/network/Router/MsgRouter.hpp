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
        /*
        const auto raw = std::span(std::data(payload), std::size(payload));
        const auto bytes = std::as_bytes(raw);
        const auto* ptr = reinterpret_cast<const uint8_t*>(bytes.data());
        return SendMessageToUser(user_id, std::span<const uint8_t>(ptr, bytes.size()));
        */
        auto bytes = std::as_bytes(std::span(payload));
        return SendMessageToUser(user_id, bytes);
    }

    bool SendMessageToUser(UserId id, std::span<const std::byte> msg) override;
    bool BroadcastToRoom(RoomId id, std::span<const std::byte> msg) override;
private:
    UserConnectionManager& connection_manager_;
};  
}
