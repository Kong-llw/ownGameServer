#include "ClientSession.hpp"

#include <asio/buffer.hpp>
#include <stdexcept>

#include "protocol/mTcpProto.h"
#include "protocol/MessageProto.hpp"

namespace Network {

ClientSession::ClientSession(SessionId session_id,
     std::shared_ptr<IMessageCodec> codec,
     std::shared_ptr<IBusinessMsgGateway> gateway)
    : session_id_(session_id), codec_(std::move(codec)), gateway_(std::move(gateway)) {
        if (!codec_) {
            throw std::invalid_argument("ClientSession requires a non-null codec");
        }
        if (!gateway_) {
            throw std::invalid_argument("ClientSession requires a non-null gateway");
        }
        read_buffer_.resize(64 * 1024); // 64KB read buffer
}

ClientSession::~ClientSession() = default;

SessionId ClientSession::GetSessionId() const {
    return session_id_;
}

void ClientSession::SetCodec(std::shared_ptr<IMessageCodec> codec) {
    if (!codec) {
        throw std::invalid_argument("SetCodec received null codec");
    }
    codec_ = std::move(codec);
}

void ClientSession::SetGateway(std::shared_ptr<IBusinessMsgGateway> gateway) {
    if (!gateway) {
        throw std::invalid_argument("SetGateway received null gateway");
    }
    gateway_ = std::move(gateway);
}

void ClientSession::SetConnection(std::shared_ptr<TcpConnection> connection) {
    if (!connection) {
        throw std::invalid_argument("SetConnection received null connection");
    }
    connection_ = std::move(connection);
}

void ClientSession::onSocketRecv(std::span<const std::byte> data) {
    if (data.empty()) {
        return;
    }

    auto decode_result = codec_->DecodeSync(data);
    if (!decode_result.success) {
        return;
    }

    for (auto& msg : decode_result.messages) {
        gateway_->onMsgReceive(msg);
    }
}

bool ClientSession::SendMessage(std::span<const std::byte> message) {
    const MsgId msg_id = NextOutboundMsgId();
    const uint8_t type = static_cast<uint8_t>(ProtoType::Data);
    Network::EncodeMessage msg{msg_id, type, 0, message, nullptr};
    Network::EncodeResult result = codec_->EncodeSync(msg);
    if(!result.success) {
        // Handle encoding error
        return false;
    }
    return WriteEncodedPayload(std::move(result.encoded_message));
}

MsgId ClientSession::NextOutboundMsgId() {
    static std::atomic<MsgId> next_id{1};
    return next_id.fetch_add(1, std::memory_order_relaxed);
}

bool ClientSession::WriteEncodedPayload(std::vector<std::byte>&& encoded_message) {
    if (!connection_ || encoded_message.empty()) {
        return false;
    }
    connection_->Send(std::move(encoded_message));
    return true;
}

} // namespace Network
