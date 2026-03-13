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

bool MsgRouter::SetRoomManager(std::weak_ptr<Game::GameRoomManager> room_manager) {
    game_room_manager_ = std::move(room_manager);
    return true;
}

bool MsgRouter::SendMessageToUser(UserId id, std::span<const std::byte> msg) {
    auto session = user_session_map_->GetSession(id);
    if (!session) {
        return false;
    }
    return session->SendMessage(msg);
}

bool MsgRouter::BroadcastToRoom(RoomId id, std::span<const std::byte> encoded_msg) {
    auto room_manager = game_room_manager_.lock();
    if (!room_manager) {
        return false;
    }
    return room_manager->RoomBroadCast(id, encoded_msg);
}

bool MsgRouter::onMsgReceive(DecodedMessage& msg) {
    auto handler_it = msg_handlers_.find(msg.main_type);
    if (handler_it == msg_handlers_.end()) {
        return false;
    }
    auto handler = handler_it->second.lock();
    if (!handler) {
        msg_handlers_.erase(handler_it);
        return false;
    }
    //后续修改为异步
    return handler->HandleDecodedMsg(msg);
}

bool MsgRouter::UnregisterMsgHandler(uint8_t main_type) {
    return msg_handlers_.erase(main_type) > 0;
}

bool MsgRouter::RegisterMsgHandler(uint8_t main_type, std::shared_ptr<IMsgHandler> handler) {
    msg_handlers_[main_type] = handler;
    return true;
}

}//namespace Network