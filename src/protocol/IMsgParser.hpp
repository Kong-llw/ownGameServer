#pragma once
#include <vector>
#include <string>
#include "core/Types.h"
#include "protocol/ProtocolType.hpp"

struct ParsedMsg {
    ProtoInfo::ProtocolType main_type;
    uint8_t sub_type;
    MsgId msg_id;
    uint8_t version;
    std::vector<std::byte> payload;
};

struct ParseResult {
    bool success;
    std::string error_msg; // 解析错误信息，成功时为空
    ParsedMsg parsed_msg;  // 解析成功时的消息内容
};

class IMsgParser {
public:
    virtual ~IMsgParser() = default;
    virtual ParseResult ParseMessage(const std::vector<std::byte>& raw_msg) = 0;
};