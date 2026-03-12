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
#include "network/session/ClientSession.hpp"
//只是起到一个转接表的功能，将用户ID和网络连接（Session）关联起来，供业务层查询
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
