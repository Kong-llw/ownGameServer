#pragma once

#include "GamePlayerInfo.hpp"
#include "protocol/PlayerProto.hpp"

class GamePlayerProtoImpl : public PlayerProtoExt<GamePlayerInfo> {
public:
    bool SerializeGameInfo(const GamePlayerInfo& game_info, std::vector<std::byte>& out) override {
        // 仅序列化游戏专属字段（seat_index/color等）
        out.push_back(static_cast<std::byte>(game_info.seat_index));
        out.push_back(static_cast<std::byte>(game_info.color));
        out.push_back(static_cast<std::byte>(game_info.position));
        out.push_back(static_cast<std::byte>(game_info.ready));
        return true;
    }

    bool DeserializeGameInfo(const std::vector<std::byte>& in, GamePlayerInfo& out) override {
        if (in.size() < 4) return false;
        out.seat_index = static_cast<uint8_t>(in[0]);
        out.color = static_cast<uint8_t>(in[1]);
        out.position = static_cast<uint8_t>(in[2]);
        out.ready = static_cast<uint8_t>(in[3]);
        return true;
    }
};