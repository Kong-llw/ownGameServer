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
    if (!session) {
        return false;
    }
    user_session_map_->Bind(user_id, session->GetSessionId());
    return true;
}

bool MsgRouter::UnregisterUserSession(UserId user_id) {
    return user_session_map_->Unbind(user_id);
}

bool MsgRouter::UnregisterSessionById(SessionId session_id) {
    return user_session_map_->UnbindBySessionId(session_id);
}

bool MsgRouter::SetSessionManager(std::weak_ptr<SessionManager> session_manager) {
    session_manager_ = std::move(session_manager);
    return true;
}

bool MsgRouter::SetRoomManager(std::weak_ptr<Game::GameRoomManager> room_manager) {
    game_room_manager_ = std::move(room_manager);
    return true;
}

bool MsgRouter::SendMessageToUser(UserId id, EncodeMessage& msg) {
    const SessionId sid = user_session_map_->GetSessionId(id);
    if (sid == SessionId{}) {
        return false;
    }
    return SendMessageToSession(sid, msg);
}

bool MsgRouter::SendMessageToSession(SessionId id, EncodeMessage& msg) {
    auto session_manager = session_manager_.lock();
    if (!session_manager) {
        return false;
    }
    auto session = session_manager->GetSession(id);
    if (!session) {
        return false;
    }
    return session->SendMessage(msg);
}

bool MsgRouter::BroadcastToRoom(RoomId id, EncodeMessage& msg) {
    auto room_manager = game_room_manager_.lock();
    if (!room_manager) {
        return false;
    }
    return room_manager->RoomBroadCast(id, msg);

}

bool MsgRouter::onMsgReceive(const std::shared_ptr<Network::MsgPack>& msg) {
    auto handler_it = msg_handlers_.find(msg->msg.main_type);
    if (handler_it == msg_handlers_.end()) {
        return false;
    }
    auto handler = handler_it->second.lock();
    if (!handler) {
        msg_handlers_.erase(handler_it);
        return false;
    }
    msg->sender_id = user_session_map_->GetUserId(msg->sender_session_id);
    //HandleDecodedMsg必须为异步实现, 避免阻塞Router
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