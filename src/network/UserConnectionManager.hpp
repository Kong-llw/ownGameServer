#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <shared_mutex>
#include <span>
#include <type_traits>
#include <unordered_map>

#include "core/Types.h"
#include "network/ClientSession.hpp"

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

class UserConnectionManager {
public:
    void Bind(UserId user_id, std::shared_ptr<ClientSession> session);
    bool Unbind(UserId user_id);
    bool UnbindBySessionId(SessionId session_id);

    std::shared_ptr<ClientSession> GetSession(UserId user_id) const;
    SessionId GetSessionId(UserId user_id) const;

    bool SendToUser(UserId user_id, std::span<const uint8_t> payload) const;

    template <ContiguousContainer C>
    bool SendToUser(UserId user_id, const C& payload) const {
        const auto raw = std::span(std::data(payload), std::size(payload));
        const auto bytes = std::as_bytes(raw);
        const auto* ptr = reinterpret_cast<const uint8_t*>(bytes.data());
        return SendToUser(user_id, std::span<const uint8_t>(ptr, bytes.size()));
    }

    std::size_t Size() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<UserId, std::shared_ptr<ClientSession>> user_to_session_;
};

} // namespace Network
