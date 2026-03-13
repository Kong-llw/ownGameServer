#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <vector>
#include "core/Types.h"

namespace Network {

using ByteSpan = std::span<const std::byte>;
using ByteVec = std::vector<std::byte>;
using SharedByteVec = std::shared_ptr<const ByteVec>;

struct EncodeMessage {
    MsgId msg_id;
    uint8_t main_type;
    uint8_t sub_type;
    ByteSpan payload;
    SharedByteVec payload_owner;
};

struct EncodeResult {
    bool success;
    std::string error_msg; // 解析错误信息，成功时为空
    ByteVec encoded_message;  // 解析成功时的消息内容
};

struct DecodedMessage {
    MsgId msg_id;
    uint8_t main_type;
    uint8_t sub_type;
    ByteVec payload;
};

struct DecodeResult {
    bool success;
    std::string error_msg; // 解析错误信息，成功时为空
    std::vector<DecodedMessage> messages;  // 解析成功时的消息内容
    size_t cost_offset;//表示应丢弃字节数 (无法解析的内容 或 已经处理完的内容)
};

class IMessageCodec {
public:
    virtual ~IMessageCodec() = default;

    virtual EncodeResult EncodeSync(EncodeMessage& msg) = 0;
    virtual DecodeResult DecodeSync(ByteSpan input) = 0;
};


class IAsyncMessageCodec {
public:
    using EncodeCallback = std::function<void(ByteVec&&)>;
    using DecodeCallback = std::function<void(DecodeResult&&)>;

    virtual ~IAsyncMessageCodec() = default;

    virtual ByteVec EncodeSync(EncodeMessage msg) = 0;
    virtual void EncodeAsync(EncodeMessage msg, EncodeCallback callback) = 0;
    virtual DecodeResult DecodeSync(ByteSpan input) = 0;
    virtual void DecodeAsync(SharedByteVec input, DecodeCallback callback) = 0;
};

class PassthroughCodec final : public IAsyncMessageCodec {
public:
    ByteVec EncodeSync(EncodeMessage msg) override {
        return ByteVec(msg.payload.begin(), msg.payload.end());
    }

    void EncodeAsync(EncodeMessage msg, EncodeCallback callback) override {
        callback(EncodeSync(std::move(msg)));
    }

    DecodeResult DecodeSync(ByteSpan input) override {
        return DecodeResult{true, "", {DecodedMessage{0, 0, 0, ByteVec(input.begin(), input.end())}}, input.size()};
    }

    void DecodeAsync(SharedByteVec input, DecodeCallback callback) override {
        callback(DecodeSync(*input));

    }
};

} // namespace Network
