//游戏流程状态机 
#pragma once

#include <asio.hpp>
#include <chrono>
#include <memory>
#include <functional>

#include "GameProto.hpp"
#include "RuntimeData.hpp"
#include "core/Types.h"
#include "business/GamePlayer/GamePlayerInfo.hpp"
#include "network/codec/IMessageCodec.hpp"
#include "business/IMsgHandler.hpp"

struct MatchInfo;
class GameCmdHandler : public IMsgHandler{ //虽然也是Handler 但每个GameRoom独有一份，在游戏开始时创建
    using GameEndCallback = std::function<void(const UserId winner_id)>;
    using ClearUserTilesCallback = std::function<void(const UserId player_id)>;
public:
    GameCmdHandler() = delete;
    GameCmdHandler(RoomId room_id, asio::any_io_executor executor, GameEndCallback end_callback);
    ~GameCmdHandler() = default;
    void StartTicking(std::chrono::milliseconds interval = std::chrono::milliseconds(50));
    void StopTicking();

    void StartGame(const MatchInfo& match_info);
    void HandlePlayerInput(UserId player_id, const std::vector<std::byte>& input_data);

    void InitGameState();//如果有需要初始化的

    void Tick(uint64_t delta_ms); //定时器驱动的状态机更新
    void ProcessCommand(const BattleCmd& cmd);
    bool HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& msg) override;
    bool Authentication(const std::shared_ptr<Network::MsgPack>& msg) override;
private:
    GameCmdHandler(const GameCmdHandler&) = delete;
    GameCmdHandler(GameCmdHandler&&) = delete;
    GameCmdHandler& operator=(GameCmdHandler&&) = delete;
    GameCmdHandler& operator=(const GameCmdHandler&) = delete;
    void ScheduleNextTick();

    //游戏逻辑相关
    void GameEndCheck();
    void ClearTilesOfPlayer(UserId player_id);
    bool ValidateCmd(const BattleCmd& cmd);

    asio::strand<asio::any_io_executor> strand_;
    std::unique_ptr<asio::steady_timer> tick_timer_;
    bool ticking_{false};
    std::chrono::milliseconds tick_interval_{50};
    std::shared_ptr<RuntimeData> runtime_;
    RoomId room_id_;
    GameEndCallback game_end_callback_;
};