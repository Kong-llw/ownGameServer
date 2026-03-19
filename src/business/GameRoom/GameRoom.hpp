#pragma once

#include <vector>
#include <memory>
#include <string>
#include <span>
#include <cstddef>
#include <asio.hpp>
#include "core/Types.h"
#include "business/Group/Group.hpp"
#include "business/GameLogic/GameProto.hpp"
#include "protocol/Router/IBusinessMsgGateway.hpp"
#include "protocol/MessageProto.hpp"
#include "business/GameLogic/GameCmdHandler.hpp"
#include "business/GamePlayer/GamePlayer.hpp"

struct GamePlayerInfo;
enum class RoomState { LOBBY = 0, RUNNING = 1 };

struct RoomInListInfo{      //客户端搜寻房间的时候，列表显示需要的信息
    std::string room_code;  //客户端识别房间用的 大写字母+数字
    std::string room_name;  //房间名称
    size_t capacity;        //房间容量
    size_t player_count;    //当前玩家数量
    RoomState state;        //房间状态
};

struct GameRoomInfo {
    RoomId room_id;
    std::string room_code;
    std::string room_name;
    std::string password;
    UserId owner_id;
    size_t capacity;        //房间容量
    size_t player_count;    //当前玩家数量
    RoomState state;        //房间状态
    std::string selected_map_path; 
};

namespace Game {
    struct PlayerInfoKeyExtractor {
        UserId operator()(const std::shared_ptr<Game::GamePlayer>& player) const { return player->GetPlayerId(); }
    };

    class GameRoom {
    public:
        GameRoom() = delete;
          template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, GameRoomInfo>>>
          GameRoom(T&& info, asio::any_io_executor executor): strand_(asio::make_strand(executor)),
            info_(std::forward<T>(info)),
            cmd_handler_(std::make_shared<GameCmdHandler>(info_.room_id, executor,
                [this](const UserId winner_id){ OnGameEnd(winner_id); })),
            group_(Group<std::shared_ptr<GamePlayer>, PlayerInfoKeyExtractor>{
                GroupInfo<std::shared_ptr<GamePlayer>>{info_.room_id, info_.capacity,{}}, PlayerInfoKeyExtractor{}}) {};

        GameRoomInfo GetAllInfo() const;
        RoomInListInfo GetInListInfo() const;
        std::vector<UserId> GetPlayersId() const;
        const std::string& GetRoomCode() const { return info_.room_code; }

        using Result = MsgProto::RoomReqResult;
        MatchInfo CreateMatchInfo(); // 游戏开始时根据创建房间信息，用于创建状态机
        Result JoinRoom(std::shared_ptr<GamePlayer> player);
        Result LeaveRoom(UserId player_id);
        Result SetReady(UserId player_id, bool ready);
        Result StartGame(UserId player_id);
        Result ChangeMap(UserId player_id, const std::string& map_path);
        Result ChangeRoomName(UserId player_id, const std::string& new_name);
        Result ChangePassword(UserId player_id, const std::string& new_password);
        Result KickPlayer(UserId player_id, UserId target_id);
        Result ChangeCapacity(UserId player_id, size_t new_capacity);
        Result DissolveRoom(UserId player_id);

        void Broadcast(Network::EncodeMessage& message);
        void SendTo(UserId playerId, Network::EncodeMessage& message);

        void SetMessageGateway(std::shared_ptr<Network::IBusinessMsgGateway> gateway) { message_gateway_ = gateway; }
    private:
        bool IsRunning() const { return info_.state == RoomState::RUNNING; }
        void SetRoomState(RoomState state) { info_.state = state; }
        void OnGameEnd(const UserId winner_id); // 游戏结束时的回调，参数是赢家的玩家ID，具体逻辑可以根据实际需求调整
        static Result MapGroupResult(GroupResult result) {
            switch (result) {
            case GroupResult::OK:
                return Result::OK;
            case GroupResult::FULL:
                return Result::FULL;
            case GroupResult::ALREADY_IN_GROUP:
                return Result::ALREADY_IN_ROOM;
            case GroupResult::NOT_IN_GROUP:
                return Result::NOT_IN_ROOM;
            case GroupResult::INVALID_CAPACITY:
                return Result::INVALID_CAPACITY;
            }
            return Result::UNKNOWN_ERROR;
        }

        asio::strand<asio::any_io_executor> strand_;
        GameRoomInfo info_;
        std::shared_ptr<GameCmdHandler> cmd_handler_;
        Group<std::shared_ptr<GamePlayer>, PlayerInfoKeyExtractor> group_;
        std::shared_ptr<Network::IBusinessMsgGateway> message_gateway_;

    };
}
