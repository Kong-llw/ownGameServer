#pragma once

#include <cstdint>
#include "network/codec/IMessageCodec.hpp"
//业务层消息转接到网络层
namespace Network {
class IMessageSender {
public:
    virtual ~IMessageSender() = default;

    virtual bool SendMessage(EncodeMessage& msg) = 0;
};
}
