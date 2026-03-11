#pragma once

#include <cstdint>
#include <span>

namespace Network {
class IMessageSender {
public:
    virtual ~IMessageSender() = default;

    virtual bool SendMessage(std::span<const uint8_t> message) = 0;
};
}
