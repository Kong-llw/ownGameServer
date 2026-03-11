#pragma once

#include <memory>
#include <span>
#include <vector>

#include "core/Types.h"
#include "IMessageSender.hpp"
#include "network/IMessageCodec.hpp"

namespace Network {
class ClientSession : public IMessageSender {
public:
    explicit ClientSession(SessionId session_id, std::shared_ptr<IMessageCodec> codec = nullptr);
    ~ClientSession() override;

    SessionId GetSessionId() const;
    bool SendMessage(std::span<const uint8_t> message) override;
    void SetCodec(std::shared_ptr<IMessageCodec> codec);

private:
    bool WriteEncodedPayload(std::vector<uint8_t>&& encoded_message);

    SessionId session_id_;
    std::shared_ptr<IMessageCodec> codec_;
};
}