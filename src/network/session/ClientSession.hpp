#pragma once

#include <atomic>
#include <memory>
#include <span>
#include <vector>

#include "core/Types.h"
#include "protocol/MessageProto.hpp"
#include "IMessageSender.hpp"
#include "network/codec/IMessageCodec.hpp"

namespace Network {
class ClientSession : public IMessageSender {
public:
    explicit ClientSession(SessionId session_id, std::shared_ptr<IMessageCodec> codec);
    ~ClientSession() override;

    SessionId GetSessionId() const;
    bool SendMessage(std::span<const std::byte> message) override;
    void SetCodec(std::shared_ptr<IMessageCodec> codec);

private:
    bool WriteEncodedPayload(std::vector<std::byte>&& encoded_message);
    static MsgId NextOutboundMsgId();

    SessionId session_id_;
    std::vector<std::byte> read_buffer_;
    std::shared_ptr<IMessageCodec> codec_;
};
}