#pragma once
#include <cstdint>
#include <span>
#include <vector>

#include "core/Types.h"
#include "protocol/MessageProto.hpp"
//消息路由器
namespace Network {
class IBusinessMsgGateway {
public:
    virtual ~IBusinessMsgGateway() = default;

    virtual bool SendMessageToUser(UserId id, EncodeMessage& msg) = 0;
    virtual bool SendMessageToSession(SessionId id, EncodeMessage& msg) = 0;
    virtual bool BroadcastToRoom(RoomId id, EncodeMessage& msg) = 0;
    virtual bool onMsgReceive(const std::shared_ptr<Network::MsgPack>& msg) = 0;
};
} // namespace Network
