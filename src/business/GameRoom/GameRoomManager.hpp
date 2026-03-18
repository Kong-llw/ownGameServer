#pragma once


#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <optional>

#include "core/Types.h"
#include "GameRoom.hpp"
#include "GameRoomUtils.hpp"

namespace Network {
class IBusinessMsgGateway;
}

namespace Game {
class GamePlayerManager;

class GameRoomManager{
public:
    explicit GameRoomManager(asio::any_io_executor executor)
     : executor_(std::move(executor)) {}

    explicit GameRoomManager(asio::any_io_executor executor, std::shared_ptr<Network::IBusinessMsgGateway> message_gateway)
     : executor_(std::move(executor)), message_gateway_(std::move(message_gateway)) {}

    ~GameRoomManager() = default;
    GameRoomManager(const GameRoomManager&) = delete;
    GameRoomManager& operator=(const GameRoomManager&) = delete;
    GameRoomManager(GameRoomManager&&) = delete;
    GameRoomManager& operator=(GameRoomManager&&) = delete;

    std::optional<GameRoomInfo> GetRoomInfo(RoomId room_id) const;
    std::optional<std::shared_ptr<GameRoom>> GetRoom(RoomId room_id) const;

    std::vector<RoomInListInfo> GetRoomList() const;
    std::optional<RoomId> RoomCodeToId(const std::string& room_code) const;
    bool RoomBroadCast(RoomId room_id, Network::EncodeMessage& message);
    std::optional<MatchInfo> GetRoomMatchInfo(RoomId room_id);
    
    using Result = MsgProto::RoomReqResult;
    Result CreateRoom(GameRoomInfo&&room_info, std::string& out_room_code);
    Result JoinRoom(RoomId room_id, UserId player_id);
    Result LeaveRoom(RoomId room_id, UserId player_id);
    Result SetReady(RoomId room_id, UserId player_id, bool ready);
    Result StartGame(RoomId room_id, UserId player_id);
    Result ChangeMap(RoomId room_id, UserId player_id, const std::string& map_path);
    Result ChangeRoomName(RoomId room_id, UserId player_id, const std::string& new_name);
    Result ChangePassword(RoomId room_id, UserId player_id, const std::string& new_password);
    Result KickPlayer(RoomId room_id, UserId player_id, UserId target_id);
    Result ChangeCapacity(RoomId room_id, UserId player_id, size_t new_capacity);
    Result DissolveRoom(RoomId room_id, UserId player_id);
    void SetPlayerManager(std::weak_ptr<GamePlayerManager> player_manager) { player_manager_ = std::move(player_manager); }
    void SetExecutor(asio::any_io_executor exec) { executor_ = std::move(exec); }

private:
    asio::any_io_executor executor_;    
    std::shared_ptr<Network::IBusinessMsgGateway> message_gateway_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<RoomId, std::shared_ptr<GameRoom>> rooms_;
    std::unordered_map<std::string, RoomId> room_code_map_; // 房间码到房间ID的映射
    std::atomic<RoomId> next_room_id_{1}; // 原子变量用于生成唯一房间ID
    std::weak_ptr<GamePlayerManager> player_manager_;
};
}