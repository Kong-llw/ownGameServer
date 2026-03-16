#include "ReqHandler.hpp"
#include "protocol/MessageProto.hpp"
#include "protocol/mTcpProto.h"
#include <cstring>

bool ReqHandler::Authentication(const std::shared_ptr<Network::MsgPack>& msg) {
    (void)msg;
    return true;
}

bool ReqHandler::HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& pack) {
    if(!Authentication(pack)){
        //写Log

        return false;
    }
    
    switch (pack->msg.main_type) {
        case static_cast<uint8_t>(MsgProto::MsgType::LOGINREQ):
            switch(pack->msg.sub_type) {
                case 0: //登录
                    asio::post(strand_, [this, pack](){
                        HandleLoginRequest(std::move(pack));
                    });
                    return true;
                case 1: //登出
                    asio::post(strand_, [this, pack](){
                        HandleLogoutRequest(std::move(pack));
                    });
                    return true;
                default:
                    return false;
            }
        case static_cast<uint8_t>(MsgProto::MsgType::ROOMREQ):
            switch(pack->msg.sub_type) {
                case 0: //加入房间
                    asio::post(strand_, [this, pack](){
                        HandleRoomJoinRequest(std::move(pack));
                    });
                    return true;
                case 1: //离开房间
                    asio::post(strand_, [this, pack](){
                        HandleRoomLeaveRequest(std::move(pack));
                    });
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

bool ReqHandler::HandleLoginRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    uint8_t size;
    int offset = 0;
    std::vector<std::byte>& payload = msg->msg.payload;
    memcpy(&size, payload.data(), sizeof(size));
    offset += sizeof(size);

    std::string username(size, '\0');
    memcpy(username.data(), payload.data() + offset, size);
    offset += size;

    memcpy(&size, payload.data() + offset, sizeof(size));
    offset += sizeof(size);

    std::string password(size, '\0');
    memcpy(password.data(), payload.data() + offset, size);
    offset += size;

    DBLoginReq req{std::move(username), std::move(password)};
    DBLoginRsp rsp = ValidateLogin(req);

    //在这里会连接UserId和SessionId
    UserLoginInfo login_info{rsp.result, rsp.user_id, msg->sender_session_id, std::move(rsp.error_msg)};
    for(auto& cb : login_callbacks_) {
        cb(login_info);
    }

    //按照UserId发送登录结果回去
    SendBackLoginResponse(msg, rsp);
    return true;
}

void ReqHandler::SendBackLoginResponse(const std::shared_ptr<Network::MsgPack>& msg, const DBLoginRsp& rsp) {
    std::vector<std::byte> payload;
    payload.push_back(static_cast<std::byte>(rsp.result));
    uint32_t user_id_net = htonl(rsp.user_id);
    payload.insert(payload.end(), reinterpret_cast<std::byte*>(&user_id_net), reinterpret_cast<std::byte*>(&user_id_net) + sizeof(user_id_net));

    uint8_t error_msg_size = static_cast<uint8_t>(rsp.error_msg.size());
    payload.push_back(static_cast<std::byte>(error_msg_size));
    auto msg_span = std::as_bytes(std::span{rsp.error_msg});
    payload.insert(payload.end(), msg_span.begin(), msg_span.end());

    Network::EncodeMessage response_msg;
    response_msg.msg_id = msg->msg.msg_id; //回包使用同样的msg_id
    response_msg.proto_type = static_cast<uint8_t>(ProtoType::Control);
    response_msg.main_type = static_cast<uint8_t>(MsgProto::MsgType::LOGINREQ);
    response_msg.sub_type = 1; //登录响应
    auto owner = std::make_shared<Network::ByteVec>(std::move(payload));
    response_msg.payload_owner = owner;
    response_msg.payload = *owner;

    gateway_->SendMessageToSession(msg->sender_session_id, response_msg);
}

DBLoginRsp ReqHandler::ValidateLogin(const DBLoginReq& req) {
    (void)req;
    //此处应调用数据库接口验证用户名密码, 这里直接模拟一个成功的返回
    DBLoginRsp rsp;
    rsp.user_id = 123; //模拟一个用户ID
    rsp.result = MsgProto::LoginResult::SUCCESS;
    return rsp;
}
bool ReqHandler::HandleLogoutRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    (void)msg;
    return true;
}
bool ReqHandler::HandleRoomJoinRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    (void)msg;
    return true;
}
bool ReqHandler::HandleRoomLeaveRequest(const std::shared_ptr<Network::MsgPack>&& msg){
    (void)msg;
    return true;
}