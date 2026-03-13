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

    virtual bool SendMessageToUser(UserId id, std::span<const std::byte> msg) = 0;
    virtual bool BroadcastToRoom(RoomId id, std::span<const std::byte> msg) = 0;
    virtual bool MsgDispatchToLogic(const std::vector<std::byte>& msg, MsgProto::MsgType type) = 0;
};
} // namespace Network
