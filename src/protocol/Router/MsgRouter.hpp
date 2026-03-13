#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>

#include "IBusinessMsgGateway.hpp"
#include "network/session/IMessageSender.hpp"
#include "UserSessionMap.hpp"

namespace Network {

template <typename C>
concept ContiguousContainer =
    std::ranges::contiguous_range<C> &&
    std::ranges::sized_range<C> &&
    requires(const C& c) {
        { std::data(c) };
        { std::size(c) } -> std::convertible_to<std::size_t>;
    } &&
    std::is_trivially_copyable_v<std::remove_cv_t<std::ranges::range_value_t<C>>>;

class MsgRouter : public IBusinessMsgGateway {
public:
    explicit MsgRouter(std::shared_ptr<UserSessionMap> user_session_map);
    ~MsgRouter() override = default;

    template <ContiguousContainer C>
    bool SendToUser(UserId user_id, const C& payload) const {
        auto bytes = std::as_bytes(std::span(payload));
        return SendMessageToUser(user_id, bytes);
    }

    bool RegisterUserSession(UserId user_id, std::shared_ptr<ClientSession> session);
    bool UnregisterUserSession(UserId user_id);
    bool UnregisterSessionById(SessionId session_id);

    bool SendMessageToUser(UserId id, std::span<const std::byte> msg) override;
    bool BroadcastToRoom(RoomId id, std::span<const std::byte> msg) override;
    bool MsgDispatchToLogic(const std::vector<std::byte>& msg, MsgProto::MsgType type) override;

private:
    std::shared_ptr<UserSessionMap> user_session_map_;
};
}
