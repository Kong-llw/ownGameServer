#pragma once

#include <cstdint>
#include <span>
//业务层消息转接到网络层
namespace Network {
class IMessageSender {
public:
    virtual ~IMessageSender() = default;

    virtual bool SendMessage(std::span<const std::byte> message) = 0;
};
}
