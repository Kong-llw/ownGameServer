#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "core/User/UserBaseInfo.hpp"

class UserBaseProto {
public:
    static bool Serialize(const UserBaseInfo& info, std::vector<std::byte>& out_buffer);
    static bool Deserialize(const std::vector<std::byte>& in_buffer, UserBaseInfo& out_info);

    // JSON序列化（调试用，解耦后协议层可灵活切换编解码方式）
    static std::string ToJson(const UserBaseInfo& info);
    static bool FromJson(const std::string& json_str, UserBaseInfo& out_info);
};

template <typename GamePlayerInfoT>
struct PlayerProtoExt {
    // 业务层实现具体游戏信息的编解码
    virtual bool SerializeGameInfo(const GamePlayerInfoT& game_info, std::vector<std::byte>& out) = 0;
    virtual bool DeserializeGameInfo(const std::vector<std::byte>& in, GamePlayerInfoT& out) = 0;
};