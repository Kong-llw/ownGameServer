#include "ClientSession.hpp"

#include <asio/buffer.hpp>
#include <stdexcept>

namespace Network {

ClientSession::ClientSession(SessionId session_id, std::shared_ptr<IMessageCodec> codec)
    : session_id_(session_id),
            codec_(std::move(codec)) {
        if (!codec_) {
                throw std::invalid_argument("ClientSession requires a non-null codec");
        }
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
    auto encoded_message = codec_->Encode(message);
    return WriteEncodedPayload(std::move(encoded_message));
}

bool ClientSession::WriteEncodedPayload(std::vector<std::byte>&& encoded_message) {
    // Hook for transport layer: place async_write queueing here.
    // In real async write, encoded_message must stay alive until handler completes.
    auto buffer_view = asio::buffer(encoded_message.data(), encoded_message.size());
    (void)buffer_view;

    return false;
}

} // namespace Network
