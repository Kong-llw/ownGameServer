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

namespace Game {
class GameRoomManager {
public:

    static GameRoomManager& GetInstance() {
        static GameRoomManager instance;
        return instance;
    }

    std::optional<GameRoomInfo> GetRoomInfo(RoomId room_id) const {
        std::shared_lock lock(mutex_);
        auto it = rooms_.find(room_id);
        if (it == rooms_.end()) {
            return std::nullopt;
        }
        return it->second->GetAllInfo();
    }

    std::optional<std::shared_ptr<GameRoom>> GetRoom(RoomId room_id) const {
        std::shared_lock lock(mutex_);
        auto it = rooms_.find(room_id);
        if (it == rooms_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    std::vector<RoomInListInfo> GetRoomList() const;
    std::optional<RoomId> RoomCodeToId(const std::string& room_code) const;
    bool RoomBroadCast(RoomId room_id, Network::EncodeMessage& message);
    MatchInfo GetRoomMatchInfo(RoomId room_id);
    
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


private:
    GameRoomManager() = default;
    ~GameRoomManager() = default;
    GameRoomManager(const GameRoomManager&) = delete;
    GameRoomManager& operator=(const GameRoomManager&) = delete;

    mutable std::shared_mutex mutex_;
    std::unordered_map<RoomId, std::shared_ptr<GameRoom>> rooms_;
    std::unordered_map<std::string, RoomId> room_code_map_; // 房间码到房间ID的映射
    std::atomic<RoomId> next_room_id_{1}; // 原子变量用于生成唯一房间ID
};
}