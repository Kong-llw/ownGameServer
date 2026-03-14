#pragma once

#include <atomic>
#include <memory>
#include <span>
#include <vector>

#include "core/Types.h"
#include "protocol/MessageProto.hpp"
#include "IMessageSender.hpp"
#include "TcpConnection.hpp"
#include "network/codec/IMessageCodec.hpp"
#include "protocol/Router/IBusinessMsgGateway.hpp"

namespace Network {
struct MsgPack{
    UserId sender_id;
    DecodedMessage msg;
};

class ClientSession : public IMessageSender {
public:
    explicit ClientSession(SessionId session_id,
         std::shared_ptr<IMessageCodec> codec,
         std::shared_ptr<IBusinessMsgGateway> gateway);
    ~ClientSession() override;

    SessionId GetSessionId() const;
    bool SendMessage(std::span<const std::byte> message) override;
    void SetCodec(std::shared_ptr<IMessageCodec> codec);
    void SetGateway(std::shared_ptr<IBusinessMsgGateway> gateway);
    void onSocketRecv(std::span<const std::byte> data);private:
    bool WriteEncodedPayload(std::vector<std::byte>&& encoded_message);
    static MsgId NextOutboundMsgId();

    SessionId session_id_;
    std::shared_ptr<TcpConnection> connection_;
    std::vector<std::byte> read_buffer_;
    std::shared_ptr<IMessageCodec> codec_;
    std::shared_ptr<IBusinessMsgGateway> gateway_; //由sessionManager注入
};
}