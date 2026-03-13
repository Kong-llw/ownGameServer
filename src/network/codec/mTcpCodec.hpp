#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <span>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "core/Types.h"
#include "core/ToolFunc.hpp"
#include "IMessageCodec.hpp"
#include "protocol/mTcpProto.h"
#include "protocol/MessageProto.hpp"

constexpr size_t CHUNK_SIZE = 4 * 1024;            // 4KB
constexpr size_t MAX_DATA_SIZE = 10 * 1024 * 1024; // 10MB

class mTcpCodec : public Network::IMessageCodec {
public:
    Network::EncodeResult EncodeSync(Network::EncodeMessage& msg) override {
        Network::EncodeResult result{};
        if (msg.payload.empty() || msg.payload.size() > MAX_DATA_SIZE) {
            result.success = false;
            result.error_msg = "TcpCodec Input Size Err, size :" + std::to_string(msg.payload.size());
            return result;
        }

        const size_t total_frames = (msg.payload.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
        if (total_frames > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
            result.success = false;
            result.error_msg = "TcpCodec too many fragments";
            return result;
        }

        std::vector<std::byte>& encoded = result.encoded_message;
        encoded.reserve(msg.payload.size() + total_frames * 
            (sizeof(ProtoHeader) + sizeof(msg.main_type) + sizeof(msg.sub_type)));

        for (size_t seq = 0; seq < total_frames; ++seq) {
            const size_t offset = seq * CHUNK_SIZE;
            const size_t remain = msg.payload.size() - offset;
            const size_t chunk_len = std::min(CHUNK_SIZE, remain);

            //获取当前包的特征Flag
            uint16_t flags = ProtoFlags::CHECKSUM;
            if (total_frames == 1) {
                flags |= ProtoFlags::FULLMSG;
            } else {
                flags |= ProtoFlags::FRAGMENTED;
                if (seq + 1 == total_frames) {
                    flags |= ProtoFlags::ENDPART;
                }
            }
            //构造当前包的内容 从span取源 把EncodeMessage中的业务类型写入前两个字节，整体作为负载内容
            const auto chunk_span = msg.payload.subspan(offset, chunk_len);
            const size_t payload_len = sizeof(msg.main_type) + sizeof(msg.sub_type) + chunk_len;

            const size_t old_size = encoded.size();
            encoded.resize(old_size + sizeof(ProtoHeader) + payload_len);

            auto* payload_start = encoded.data() + old_size + sizeof(ProtoHeader);
            payload_start[0] = static_cast<std::byte>(msg.main_type);
            payload_start[1] = static_cast<std::byte>(msg.sub_type);
            std::memcpy(payload_start + sizeof(msg.main_type) + sizeof(msg.sub_type), chunk_span.data(), chunk_len);

            const uint32_t crc = Crc32C(Network::ByteSpan(payload_start, payload_len));

            //把crc结果填入header 并写入header
            ProtoHeader hdr{};
            hdr.header = htonl(PROTO_MAGIC);
            hdr.version = ProtoFlags::VERSION;
            hdr.proto_type = static_cast<uint8_t>(ProtoType::Data);
            hdr.payload_len = htons(static_cast<uint16_t>(payload_len));
            hdr.flags = htons(flags);
            hdr.msg_id_high = htonl(static_cast<uint32_t>(msg.msg_id >> 32));
            hdr.msg_id_low = htonl(static_cast<uint32_t>(msg.msg_id & 0xFFFFFFFFu));
            hdr.seq = htons(static_cast<uint16_t>(seq));
            hdr.total = htons(static_cast<uint16_t>(total_frames));
            hdr.checksum = htonl(crc);
            std::memcpy(encoded.data() + old_size, &hdr, sizeof(ProtoHeader));

            result.success = true;
        }

        return result;
    }

    Network::DecodeResult DecodeSync(std::span<const std::byte> input) override {
        Network::DecodeResult result{};
        uint32_t transedMagic = htonl(PROTO_MAGIC);
        const auto find_next_magic_offset = [&](size_t start) -> size_t {
            if (input.size() < sizeof(uint32_t) || start >= input.size()) {
                return input.size();
            }
            for (size_t i = start; i + sizeof(uint32_t) <= input.size(); ++i) {
                uint32_t magic = 0;
                std::memcpy(&magic, input.data() + i, sizeof(magic));
                if (magic == transedMagic) {
                    return i;
                }
            }
            return input.size();
        };

        if (input.size() < sizeof(ProtoHeader)) {
            result.success = false;
            result.error_msg = "TcpCodec Decode Input Too Short";
            result.cost_offset = 0;
            return result;
        }

        size_t offset = 0;
        while (offset + sizeof(ProtoHeader) <= input.size()) {
            const auto* hdr = reinterpret_cast<const ProtoHeader*>(input.data() + offset);
            if (ntohl(hdr->header) != PROTO_MAGIC) {
                const size_t magic_offset = find_next_magic_offset(offset + 1);
                result.error_msg = "TcpCodec Decode Invalid Magic";
                result.cost_offset = magic_offset;
                return result;
            }

            const size_t payload_len = ntohs(hdr->payload_len);
            const size_t frame_len = sizeof(ProtoHeader) + payload_len;
            if (offset + frame_len > input.size()) {
                result.error_msg = "TcpCodec Decode Input Too Short For Payload";
                result.cost_offset = offset;
                return result;
            }
            Network::DecodedMessage msg{};
            const size_t prefix_size = sizeof(msg.main_type) + sizeof(msg.sub_type);
            msg.msg_id = (static_cast<MsgId>(ntohl(hdr->msg_id_high)) << 32) | ntohl(hdr->msg_id_low);
            memcpy(&msg.main_type, input.data() + offset + sizeof(ProtoHeader), sizeof(msg.main_type));
            memcpy(&msg.sub_type, input.data() + offset + sizeof(ProtoHeader) + sizeof(msg.main_type), sizeof(msg.sub_type));
            if (payload_len > prefix_size) {
                msg.payload.resize(payload_len - prefix_size);
                std::memcpy(msg.payload.data(), input.data() + offset + sizeof(ProtoHeader) + prefix_size, msg.payload.size());
            }
            result.messages.push_back(std::move(msg));

            offset += frame_len;
        }
        result.success = true;
        result.cost_offset = offset;
        return result;
    }

    std::vector<std::byte> createHeartbeatFrame() {
        std::vector<std::byte> rt(sizeof(ProtoHeader));
        ProtoHeader hdr{};
        hdr.header = htonl(PROTO_MAGIC);
        hdr.version = ProtoFlags::VERSION;
        hdr.proto_type = static_cast<uint8_t>(ProtoType::HeartBeat);
        std::memcpy(rt.data(), &hdr, sizeof(ProtoHeader));
        return rt;
    }

private:
};
