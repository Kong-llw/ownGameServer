#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>
#include <unordered_map>

#include "IBusinessMsgGateway.hpp"
#include "network/session/IMessageSender.hpp"
#include "network/session/SessionManager.hpp"
#include "network/codec/mTcpCodec.hpp"
#include "business/IMsgHandler.hpp"
#include "UserSessionMap.hpp"
#include "business/GameRoom/GameRoomManager.hpp"

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
/*
    template <ContiguousContainer C>
    bool SendToUser(UserId user_id, const C& payload) const {
        auto bytes = std::as_bytes(std::span(payload));
        EncodeMessage msg;
        msg.payload = std::vector<std::byte>(bytes.begin(), bytes.end());
        return SendMessageToUser(user_id, msg);
    }*/

    bool RegisterUserSession(UserId user_id, std::shared_ptr<ClientSession> session);
    bool UnregisterUserSession(UserId user_id);
    bool UnregisterSessionById(SessionId session_id);
    bool SetSessionManager(std::weak_ptr<SessionManager> session_manager);
    bool SetRoomManager(std::weak_ptr<Game::GameRoomManager> room_manager);

    bool SendMessageToUser(UserId id, EncodeMessage& msg) override;
    bool SendMessageToSession(SessionId id, EncodeMessage& msg) override;
    bool BroadcastToRoom(RoomId id, EncodeMessage& msg) override;

    bool onMsgReceive(const std::shared_ptr<Network::MsgPack>& msg) override;
    bool RegisterMsgHandler(uint8_t main_type, std::shared_ptr<IMsgHandler> handler);
    bool UnregisterMsgHandler(uint8_t main_type);
private:
    //服务->客户端(session) 接收已经编码的业务消息, 转发到对应session
    std::shared_ptr<UserSessionMap> user_session_map_;
    std::weak_ptr<SessionManager> session_manager_;
    std::weak_ptr<Game::GameRoomManager> game_room_manager_;

    //客户端->服务 识别已经解码的消息类型 并交给对应的消息处理器
    std::unordered_map<uint8_t, std::weak_ptr<IMsgHandler>> msg_handlers_; // Header.Msgtype -> handler
};
}
