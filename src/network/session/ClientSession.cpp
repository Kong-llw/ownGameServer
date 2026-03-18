#include "ClientSession.hpp"

#include <asio/buffer.hpp>
#include <cstring>
#include <stdexcept>
#include <spdlog/spdlog.h>

#include "protocol/mTcpProto.h"
#include "protocol/MessageProto.hpp"

namespace Network {

ClientSession::ClientSession(SessionId session_id,
     std::shared_ptr<IMessageCodec> codec,
     std::shared_ptr<IBusinessMsgGateway> gateway)
    : session_id_(session_id), buffer_used_(0), codec_(std::move(codec)), gateway_(std::move(gateway)) {
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
    /*if (buffer_used_ + data.size() > read_buffer_.size()) {
        // Buffer overflow, close connection to prevent DoS
        Close();
        return;
    }*/
    std::copy(data.begin(), data.end(), read_buffer_.begin() + buffer_used_);
    buffer_used_ += data.size();
    
    auto decode_result = codec_->DecodeSync(std::span<const std::byte>(read_buffer_.data(), buffer_used_));
    spdlog::info("Decoded message from session {}: success={}, cost_offset={}, err_messages={}", 
        session_id_, decode_result.success, decode_result.cost_offset, decode_result.error_msg);
    if (decode_result.cost_offset > 0) {
        // Move remaining data to the front of the buffer
        std::memmove(read_buffer_.data(), read_buffer_.data() + decode_result.cost_offset, buffer_used_ - decode_result.cost_offset);
        buffer_used_ -= decode_result.cost_offset;
    }
    
    if (!decode_result.success) {
        return;
    }

    for(auto& msg : decode_result.messages) {
        auto pack_ptr = std::make_shared<MsgPack>();
        pack_ptr->sender_session_id = session_id_;
        pack_ptr->msg = std::move(msg);
        gateway_->onMsgReceive(pack_ptr);
    }
}

bool ClientSession::SendMessage(EncodeMessage& msg) {
    if (msg.msg_id == MsgId{}) { //此分支是主动发, 回应包时msg_id由业务层指定，主动发时由session生成唯一msg_id
        msg.msg_id = NextOutboundMsgId();
    }

    Network::EncodeResult result = codec_->EncodeSync(msg);
    if(!result.success) {
        // Handle encoding error
        return false;
    }
    return WriteEncodedPayload(std::move(result.encoded_message));
}

void ClientSession::Close() {
    if (connection_) {
        connection_->Close();
    }
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
