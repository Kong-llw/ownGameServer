#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Network {

class IMessageCodec {
public:
    virtual ~IMessageCodec() = default;

    // Encode payload before transport write (e.g. framing/encryption/compression).
    virtual std::vector<uint8_t> Encode(std::span<const uint8_t> payload) = 0;
};

class PassthroughCodec final : public IMessageCodec {
public:
    std::vector<uint8_t> Encode(std::span<const uint8_t> payload) override {
        return std::vector<uint8_t>(payload.begin(), payload.end());
    }
};

} // namespace Network
