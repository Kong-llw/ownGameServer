#pragma once

#include <cstdint>
#include <cstddef>

constexpr size_t PROTO_HEADER_SIZE = 4 + 1 + 1 + 2 + 2 + 4 + 4 + 2 + 2 + 4;
constexpr uint32_t PROTO_MAGIC = 0x48123123; //协议魔数

namespace ProtoFlags{
    constexpr uint8_t VERSION = 1;
    constexpr uint16_t SHORTMSG = 0x0;
    constexpr uint16_t FRAGMENTED = 0x1;
    constexpr uint16_t CHECKSUM = 0x2;
    constexpr uint16_t ENDPART = 0x4;   //最后一个特殊包，保存整条信息的
}

#pragma pack(push, 1)
struct ProtoHeader {    //内容会以网络字节序写入
    uint32_t header; // = 0x4812 3123
    uint8_t version;
    uint8_t ptype;
    uint16_t payload_len;
    uint16_t flags; //ProtoFlags
    uint32_t msg_id_high;
    uint32_t msg_id_low;
    uint16_t seq;
    uint16_t total;
    uint32_t checksum;
};
#pragma pack(pop)

static_assert(sizeof(ProtoHeader) == PROTO_HEADER_SIZE, "ProtoHeader size mismatch");
