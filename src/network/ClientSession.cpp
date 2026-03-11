#include "network/ClientSession.hpp"

#include <asio/buffer.hpp>

namespace Network {

ClientSession::ClientSession(SessionId session_id, std::shared_ptr<IMessageCodec> codec)
    : session_id_(session_id),
      codec_(codec ? std::move(codec) : std::make_shared<PassthroughCodec>()) {}

ClientSession::~ClientSession() = default;

SessionId ClientSession::GetSessionId() const {
    return session_id_;
}

void ClientSession::SetCodec(std::shared_ptr<IMessageCodec> codec) {
    if (codec) {
        codec_ = std::move(codec);
    }
}

bool ClientSession::SendMessage(std::span<const uint8_t> message) {
    if (!codec_) {
        codec_ = std::make_shared<PassthroughCodec>();
    }

    auto encoded_message = codec_->Encode(message);
    return WriteEncodedPayload(std::move(encoded_message));
}

bool ClientSession::WriteEncodedPayload(std::vector<uint8_t>&& encoded_message) {
    // Hook for transport layer: place async_write queueing here.
    // In real async write, encoded_message must stay alive until handler completes.
    auto buffer_view = asio::buffer(encoded_message.data(), encoded_message.size());
    (void)buffer_view;

    return false;
}

} // namespace Network
