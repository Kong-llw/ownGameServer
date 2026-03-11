#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Network {

class IMessageCodec {
public:
    virtual ~IMessageCodec() = default;

    // Encode payload before transport write (e.g. framing/encryption/compression).
    virtual std::vector<std::byte> Encode(std::span<const std::byte> payload) = 0;
};

class PassthroughCodec final : public IMessageCodec {
public:
    std::vector<std::byte> Encode(std::span<const std::byte> payload) override {
        return std::vector<std::byte>(payload.begin(), payload.end());
    }
};

} // namespace Network
