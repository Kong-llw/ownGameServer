#pragma once

#include "GamePlayerInfo.hpp"
#include "protocol/PlayerProto.hpp"

class GamePlayerProtoImpl : public PlayerProtoExt<GamePlayerInfo> {
public:
    bool SerializeGameInfo(const GamePlayerInfo& game_info, std::vector<uint8_t>& out) override {
        // 仅序列化游戏专属字段（seat_index/color等）
        out.push_back(game_info.seat_index);
        out.push_back(game_info.color);
        out.push_back(game_info.position);
        out.push_back(game_info.ready);
        return true;
    }

    bool DeserializeGameInfo(const std::vector<uint8_t>& in, GamePlayerInfo& out) override {
        if (in.size() < 4) return false;
        out.seat_index = in[0];
        out.color = in[1];
        out.position = in[2];
        out.ready = in[3];
        return true;
    }
};