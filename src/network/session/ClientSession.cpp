#include "ClientSession.hpp"

#include <asio/buffer.hpp>
#include <stdexcept>

#include "protocol/mTcpProto.h"
#include "protocol/MessageProto.hpp"

namespace Network {

ClientSession::ClientSession(SessionId session_id, std::shared_ptr<IMessageCodec> codec)
    : session_id_(session_id),
            codec_(std::move(codec)) {
        if (!codec_) {
            throw std::invalid_argument("ClientSession requires a non-null codec");
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
    // Hook for transport layer: place async_write queueing here.
    // In real async write, encoded_message must stay alive until handler completes.
    auto buffer_view = asio::buffer(encoded_message.data(), encoded_message.size());
    (void)buffer_view;

    return false;
}

} // namespace Network
