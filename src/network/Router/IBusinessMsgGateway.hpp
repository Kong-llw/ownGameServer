#pragma once
#include <cstdint>
#include <span>
#include "core/Types.h"
//消息路由器
namespace Network {
class IBusinessMsgGateway {
public:
    virtual ~IBusinessMsgGateway() = default;

    virtual bool SendMessageToUser(UserId id, std::span<const std::byte> msg) = 0;
    virtual bool BroadcastToRoom(RoomId id, std::span<const std::byte> msg) = 0;
};
}