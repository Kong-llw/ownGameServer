#include "GameCmdHandler.hpp"
#include "GameMapIO.hpp"
#include <utility>

#include "GameProto.hpp"

GameCmdHandler::GameCmdHandler(RoomId room_id, asio::any_io_executor executor, GameEndCallback end_callback)
    : strand_(asio::make_strand(executor)),tick_timer_(std::make_unique<asio::steady_timer>(strand_)),
     room_id_(room_id), game_end_callback_(std::move(end_callback)) {
    InitGameState();
}

void GameCmdHandler::StartTicking(std::chrono::milliseconds interval) {
    if (ticking_) {
        return;
    }
    tick_interval_ = interval;
    ticking_ = true;

    if (!tick_timer_) {
        tick_timer_ = std::make_unique<asio::steady_timer>(strand_);
    }
    ScheduleNextTick();
}

void GameCmdHandler::StopTicking() {
    ticking_ = false;
    if (tick_timer_) {
        tick_timer_->cancel();
    }
}

void GameCmdHandler::StartGame(const MatchInfo& match_info) {
    if (!runtime_) {
        runtime_ = std::make_shared<RuntimeData>();
    }
    runtime_->map_name = match_info.selected_map_path;
    const MapData* map_data = GameMap::LoadMapData(match_info.selected_map_path);
    if (map_data) {
        runtime_->map_data = *map_data;
    }else{
        throw std::runtime_error("Failed to load map data for: " + match_info.selected_map_path);
    }
    runtime_->current_player_id = match_info.owner_id;
    runtime_->tick = 0;

    for (std::size_t i = 0; i < match_info.players.size(); ++i) {
        const auto& player = match_info.players[i];
        runtime_->players[player.b_info.user_id] = PlayerRuntimeState{
            player.b_info.user_id,
            static_cast<uint32_t>(i),
            0,
            0,
            0,
            false,
        };
    }
}

void GameCmdHandler::HandlePlayerInput(UserId player_id, const std::vector<std::byte>& input_data) {
    (void)player_id;
    (void)input_data;
}

void GameCmdHandler::InitGameState() {
    if (!runtime_) {
        runtime_ = std::make_shared<RuntimeData>();
    }
    runtime_->players.clear();
}

void GameCmdHandler::Tick(uint64_t delta_ms) {
    if (!runtime_) {
        return;
    }
    runtime_->tick += delta_ms;
}

void GameCmdHandler::ProcessCommand(const BattleCmd& cmd) {
    if(!ValidateCmd(cmd)){
        throw std::runtime_error("invalid command");
    }
    switch (cmd.type) {
        case BattleCmdType::Move:{
            TileData& src = runtime_->map_data.tiles[cmd.param1];
            TileData& dst = runtime_->map_data.tiles[cmd.param2];
            src.value = 1;
            src.acted = true;

            dst.owner = src.owner;
            dst.value += src.value - 1;

            //玩家数据更新
            auto& player_state = runtime_->players[cmd.player_id];
            player_state.resources += 1; // 占位：每次移动增加资源，具体规则可调整
            if(dst.type == TileType::Base) {
                player_state.baseCount += 1; // 占领新基地
            }
            break;
        }
        case BattleCmdType::Attack:{
            TileData& at_src = runtime_->map_data.tiles[cmd.param1];
            TileData& at_dst = runtime_->map_data.tiles[cmd.param2];
            auto& player_state1 = runtime_->players[at_src.owner];
            auto& player_state2 = runtime_->players[at_dst.owner];
            if(at_src.value > at_dst.value){ //胜利部分
                at_src.value = 1;
                at_dst.value = at_dst.value - at_src.value - 1; // TODO: 根据游戏规则完善结算逻辑
                if(at_dst.value > 0){
                    at_dst.owner = at_src.owner;
                    player_state1.resources+=1;
                    player_state2.resources-=1;
                }    
                else{
                    at_dst.owner = 0;
                    player_state2.resources-=1;
                }
                if (at_dst.type == TileType::Base){
                    if(at_dst.owner == at_src.owner){
                        player_state1.baseCount += 1; // 占领新基地
                    }
                    player_state2.baseCount -= 1; // 失去基地
                    GameEndCheck();
                }
            } else {
                if(at_dst.countered == false){ //受反击被消灭
                    at_dst.countered = true;
                    at_src.value = 0;
                    at_src.owner = 0;
                    player_state1.resources-=1;
                    if (at_src.type == TileType::Base){
                        player_state1.baseCount -= 1; // 失去基地
                        GameEndCheck();
                    }
                }
                else {
                    at_src.acted = true;
                }

                at_dst.value -= at_src.value;
                if(at_dst.value == 0){  // 两方同归于尽
                    at_dst.owner = 0;
                    player_state1.resources-=1; 
                    player_state2.resources-=1;
                    if (at_dst.type == TileType::Base){
                        player_state2.baseCount -= 1; // 失去基地
                        GameEndCheck(); 
                    }
                } 
            }
            break;
        }
        case BattleCmdType::Grow:{  
            TileData& dst = runtime_->map_data.tiles[cmd.param1];
            dst.value += cmd.param2; // TODO: 根据游戏规则完善生长逻辑
            break;
        }
        default:    
            throw std::runtime_error("unknown command type");
    }
}

bool GameCmdHandler::HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& msg) {
    (void)msg;
    return true;
}

bool GameCmdHandler::Authentication(const std::shared_ptr<Network::MsgPack>& msg) {
    (void)msg;
    return true;
}

void GameCmdHandler::ScheduleNextTick() {
    if (!ticking_ || !tick_timer_) {
        return;
    }

    tick_timer_->expires_after(tick_interval_);
    tick_timer_->async_wait([this](const std::error_code& ec) {
        if (ec || !ticking_) {
            return;
        }
        Tick(static_cast<uint64_t>(tick_interval_.count()));
        ScheduleNextTick();
    });
}

void GameCmdHandler::GameEndCheck() {
    bool SinglePlayerLeft = true;
    uint64_t lastPlayerId = 0;
    for (auto& [player_id, player_state] : runtime_->players) {
        if (player_state.failure) {
            if (lastPlayerId != 0 && lastPlayerId != player_id) {
                SinglePlayerLeft = false;
            }
            lastPlayerId = player_id;
        }
        else {
            continue;
        }

        if (player_state.baseCount == 0) {
            player_state.failure = true;
            ClearTilesOfPlayer(player_id); // 占位：玩家被淘汰后清除其格子占领状态，具体规则可调整  
        }
    }

    if(SinglePlayerLeft && game_end_callback_) {
	    game_end_callback_(lastPlayerId);
    }
}

void GameCmdHandler::ClearTilesOfPlayer(UserId player_id) {
    //鉴定功能上浮到room，由room判断目前是否在游戏中
    for (TileData& tile : runtime_->map_data.tiles) {
        if (tile.owner == player_id) {
            tile.owner = 0;
            tile.value = 0;
            tile.countered = false;
            tile.acted = false;
        }
    }

}

bool GameCmdHandler::ValidateCmd(const BattleCmd& cmd) {
    switch (cmd.type){
        case BattleCmdType::Move: {
            const TileData& src = runtime_->map_data.tiles.at(cmd.param1);
            const TileData& dst = runtime_->map_data.tiles.at(cmd.param2);
            return cmd.param1 != cmd.param2
                && src.owner == cmd.player_id
                && dst.owner == cmd.player_id
                && src.value > 1; // TODO: 根据游戏规则完善验证逻辑
        }
        case BattleCmdType::Attack: {
            const TileData& at_src = runtime_->map_data.tiles.at(cmd.param1);
            const TileData& at_dst = runtime_->map_data.tiles.at(cmd.param2);
            return cmd.param1 != cmd.param2
                && at_src.owner == cmd.player_id
                && at_dst.owner != cmd.player_id
                && at_src.value > 1; // TODO: 根据游戏规则完善验证逻辑
        }
        case BattleCmdType::Grow: {
            const TileData& dst = runtime_->map_data.tiles.at(cmd.param1);
            return cmd.player_id == dst.owner
                && dst.value > 0; // TODO: 根据游戏规则完善验证逻辑
        }
        default: return false;
    }
    return true;
}
