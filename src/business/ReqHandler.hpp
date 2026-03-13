//登陆退出 进出房间之类的消息处理  二次转发到具体执行类

#pragma once
#include <vector>
#include "protocol/MessageProto.hpp"
#include "network/codec/IMessageCodec.hpp"
#include "business/IMsgHandler.hpp"

class ReqHandler : public IMsgHandler {
public:
    bool HandleDecodedMsg(Network::DecodedMessage& msg) override;
private:
    bool HandleLoginRequest(Network::DecodedMessage& msg);
    bool HandleLogoutRequest(Network::DecodedMessage& msg);
    bool HandleRoomJoinRequest(Network::DecodedMessage& msg);
    bool HandleRoomLeaveRequest(Network::DecodedMessage& msg);
};