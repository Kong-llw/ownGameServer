//游戏流程状态机 
#pragma once

#include <memory>
#include <functional>

#include "GameProto.hpp"
#include "RuntimeData.hpp"
#include "core/Types.h"
#include "business/GamePlayer/GamePlayerInfo.hpp"
#include "network/codec/IMessageCodec.hpp"
#include "business/IMsgHandler.hpp"

struct MatchInfo;
class GameCmdHandler : public IMsgHandler{
    using GameEndCallback = std::function<void(const UserId winner_id, const RoomId room_id)>;
    using ClearUserTilesCallback = std::function<void(const UserId player_id)>;
public:
    GameCmdHandler(const MatchInfo& match_info, 
        GameEndCallback end_callback, ClearUserTilesCallback clear_tiles_callback);
    ~GameCmdHandler() = default;
    void StartGame();
    void HandlePlayerInput(UserId player_id, const std::vector<std::byte>& input_data);

    void InitGameState(const MatchInfo& match_info); //考虑到有磁盘操作,将初始化逻辑与构造分开

    void Tick(uint64_t delta_ms); //定时器驱动的状态机更新
    void ProcessCommand(const BattleCmd& cmd);
    bool HandleDecodedMsg(Network::DecodedMessage& msg) override;
private:
    GameCmdHandler(const GameCmdHandler&) = delete;
    GameCmdHandler(GameCmdHandler&&) = delete;
    GameCmdHandler& operator=(GameCmdHandler&&) = delete;
    GameCmdHandler& operator=(const GameCmdHandler&) = delete;
    //游戏逻辑相关
    void GameEndCheck();
    void ClearTilesOfPlayer(UserId player_id);
    bool ValidateCmd(const BattleCmd& cmd);
    RuntimeData runtime_;
    GameEndCallback game_end_callback_;
    ClearUserTilesCallback clear_user_tiles_callback_;
};