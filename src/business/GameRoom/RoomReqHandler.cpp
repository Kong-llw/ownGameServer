#include "RoomReqHandler.hpp"
#include "protocol/MessageProto.hpp"
#include "protocol/mTcpProto.h"
#include <cstring>
#include "protocol/JsonTranslator.hpp"
namespace Game{

bool RoomReqHandler::Authentication(const std::shared_ptr<Network::MsgPack>& msg) {
    (void)msg;
    return true;
}

bool RoomReqHandler::HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& pack) {
    if(!Authentication(pack)){
        //写Log

        return false;
    }
    
    switch (pack->msg.main_type) {
        case static_cast<uint8_t>(MsgProto::MsgType::ROOMREQ):
            switch(pack->msg.sub_type) {//RoomReqType
                case 1: //创建房间
                    asio::post(strand_, [this, pack](){
                        HandleRoomCreateRequest(std::move(pack));
                    });
                    return true;
                case 2: //加入房间
                    asio::post(strand_, [this, pack](){
                        HandleRoomJoinRequest(std::move(pack));
                    });
                    return true;
                case 3: //离开房间
                    asio::post(strand_, [this, pack](){
                        HandleRoomLeaveRequest(std::move(pack));
                    });
                    return true;
                case 4: //列出存在的房间
                    asio::post(strand_, [this, pack](){
                        HandleRoomListRequest(std::move(pack));
                    });
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

bool RoomReqHandler::HandleRoomJoinRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    std::string room_code;
    memcpy(room_code.data(), msg->msg.payload.data(), 6);
    std::optional<RoomId> room_id = game_room_manager_->RoomCodeToId(room_code);
    std::vector<std::byte> payload;
    Network::EncodeMessage response_msg;
    response_msg.msg_id = msg->msg.msg_id; //回包使用同样的msg_id
    response_msg.proto_type = static_cast<uint8_t>(ProtoType::Control);
    response_msg.main_type = static_cast<uint8_t>(MsgProto::MsgType::ROOMRSP);
    response_msg.sub_type = static_cast<uint8_t>(MsgProto::RoomReqType::JOINROOM); //登录响应
    //失败部分 无房间
    if(!room_id){
        payload.push_back(static_cast<std::byte>(MsgProto::RoomReqResult::NOT_FOUND));
        response_msg.payload = payload;
        response_msg.payload_owner = std::make_shared<Network::ByteVec>(payload);
        gateway_->SendMessageToUser(msg->sender_id, response_msg);
        return false;
    }

    MsgProto::RoomReqResult out_res = game_room_manager_->JoinRoom(room_id.value(), msg->sender_id);
    payload.push_back(static_cast<std::byte>(out_res));
    
    if(out_res == MsgProto::RoomReqResult::OK){
        std::optional<MatchInfo> match_info = game_room_manager_->GetRoomMatchInfo(room_id.value());
        if(match_info){
            std::string jsonData = JSONTranslator::serializeRoomInfo(match_info.value());
            std::vector<std::byte> bytes(jsonData.size());
            memcpy(bytes.data(), jsonData.data(), jsonData.size());
            payload.insert(payload.end(), bytes.begin(), bytes.end());
        }
    }
    response_msg.payload = payload;
    response_msg.payload_owner = std::make_shared<Network::ByteVec>(payload);
    gateway_->SendMessageToUser(msg->sender_id, response_msg);

    //玩家加入广播信息
    if(out_res == MsgProto::RoomReqResult::OK){
        std::string text = "NewPlayer Joined Room";
        std::vector<std::byte> bytes(text.size());
        memcpy(bytes.data(), text.data(), text.size());
        Network::EncodeMessage newPlayerJoinBoradcastMsg;{
            newPlayerJoinBoradcastMsg.msg_id = 0, //后续由sender生成并注入
            newPlayerJoinBoradcastMsg.proto_type = static_cast<uint8_t>(ProtoType::Control),
            newPlayerJoinBoradcastMsg.main_type = static_cast<uint8_t>(MsgProto::MsgType::CHATMSG),
            newPlayerJoinBoradcastMsg.sub_type = 0, //玩家加入房间通知
            newPlayerJoinBoradcastMsg.payload = bytes,
            newPlayerJoinBoradcastMsg.payload_owner = std::make_shared<std::vector<std::byte>>(std::move(bytes));
        };
        game_room_manager_->RoomBroadCast(room_id.value(), newPlayerJoinBoradcastMsg);
    }
    return true;
}
bool RoomReqHandler::HandleRoomLeaveRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    std::string room_code;
    memcpy(room_code.data(), msg->msg.payload.data(), 6);
    std::optional<RoomId> room_id = game_room_manager_->RoomCodeToId(room_code);
    if (!room_id) return false;

    MsgProto::RoomReqResult out_res = game_room_manager_->LeaveRoom(room_id.value(), msg->sender_id);
    std::vector<std::byte> bytes = {static_cast<std::byte>(out_res)};
    Network::EncodeMessage response;{
        response.msg_id = msg->sender_id, //后续由sender生成并注入
        response.proto_type = static_cast<uint8_t>(ProtoType::Control),
        response.main_type = static_cast<uint8_t>(MsgProto::MsgType::ROOMRSP),
        response.sub_type = static_cast<uint8_t>(MsgProto::RoomReqType::LEAVEROOM), //玩家离开房间通知
        response.payload = bytes,
        response.payload_owner = std::make_shared<std::vector<std::byte>>(std::move(bytes));
    };
    gateway_->SendMessageToUser(msg->sender_id, response);
    return true;
}

bool RoomReqHandler::HandleRoomCreateRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    //读取payload请求数据
    uint8_t size = static_cast<uint8_t>(msg->msg.payload[0]);
    std::string room_name(size, '\0');
    memcpy(room_name.data(), msg->msg.payload.data() + 1, size);

    size = static_cast<uint8_t>(msg->msg.payload[1 + room_name.size()]);
    std::string password(size, '\0');
    memcpy(password.data(), msg->msg.payload.data() + 1 + room_name.size() + 1, size);

    //构造房间
    std::string out_room_code;
    GameRoomInfo temp {};
    temp.room_name = room_name;
    temp.password = password;
    temp.owner_id = msg->sender_id;
    temp.capacity = 4;
    MsgProto::RoomReqResult out_res;
    out_res = game_room_manager_->CreateRoom(std::move(temp), out_room_code);

    //根据房间创建结果，构造回复信息
    std::string str = JSONTranslator::serializeCreateRoomResult(out_room_code, out_res);
    std::vector<std::byte> payload(str.size());
    memcpy(payload.data(), str.data(), str.size());
    auto temp_owner = std::make_shared<Network::ByteVec>(std::move(payload));
    Network::EncodeMessage msgPkt{
        .msg_id = msg->msg.msg_id, //回包使用同样的msg_id
        .proto_type = static_cast<uint8_t>(ProtoType::Data),
        .main_type = static_cast<uint8_t>(MsgProto::MsgType::ROOMRSP),
        .sub_type = static_cast<uint8_t>(MsgProto::RoomReqType::CREATEROOM), //登录响应
        .payload = *temp_owner,
        .payload_owner = std::move(temp_owner)
    };
    gateway_->SendMessageToUser(msg->sender_id, msgPkt);
    return true;
}

bool RoomReqHandler::HandleRoomListRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    std::string str = JSONTranslator::serializeRoomList(game_room_manager_->GetRoomList());
    std::vector<std::byte> payload(str.size());
    memcpy(payload.data(), str.data(), str.size());
    Network::EncodeMessage response_msg;
    response_msg.msg_id = msg->msg.msg_id; //回包使用同样的msg_id
    response_msg.proto_type = static_cast<uint8_t>(ProtoType::Control);
    response_msg.main_type = static_cast<uint8_t>(MsgProto::MsgType::ROOMRSP);
    response_msg.sub_type = static_cast<uint8_t>(MsgProto::RoomReqType::LISTROOMS); //登录响应
    auto owner = std::make_shared<Network::ByteVec>(std::move(payload));
    response_msg.payload_owner = owner;
    response_msg.payload = *owner;

    gateway_->SendMessageToSession(msg->sender_session_id, response_msg);
    return true;
}

}