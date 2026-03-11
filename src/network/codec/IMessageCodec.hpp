#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Network {

class IMessageCodec {
public:
    virtual ~IMessageCodec() = default;

    // Encode payload before transport write (e.g. framing/encryption/compression).
    virtual std::vector<uint8_t> Encode(std::std::span<const std::byte> payload) = 0;
};

class PassthroughCodec final : public IMessageCodec {
public:
    std::vector<uint8_t> Encode(std::std::span<const std::byte> payload) override {
        return std::vector<uint8_t>(payload.begin(), payload.end());
    }
};

} // namespace Network
