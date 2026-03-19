#include "GameRoomManager.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <asio.hpp>
#include "GameRoom.hpp"
#include "GameRoomUtils.hpp"
#include "business/GamePlayer/GamePlayerManager.hpp"
#include "protocol/MessageProto.hpp"

namespace Game{
std::optional<GameRoomInfo> GameRoomManager::GetRoomInfo(RoomId room_id) const {
    std::shared_lock lock(mutex_);
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        return std::nullopt;
    }
    return it->second->GetAllInfo();
}

std::optional<std::shared_ptr<GameRoom>> GameRoomManager::GetRoom(RoomId room_id) const {
    std::shared_lock lock(mutex_);
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<RoomInListInfo> GameRoomManager::GetRoomList() const{
    std::shared_lock lock(mutex_);
    std::vector<RoomInListInfo> room_list;
    for (const auto& [room_id, room] : rooms_) {
        room_list.push_back(room->GetInListInfo());
    }
    return room_list;
}

std::optional<RoomId> GameRoomManager::RoomCodeToId(const std::string& room_code) const {
    std::shared_lock lock(mutex_);
    auto it = room_code_map_.find(room_code);
    if (it == room_code_map_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool GameRoomManager::RoomBroadCast(RoomId room_id, Network::EncodeMessage& message) {
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return false;
    }
    room_opt.value()->Broadcast(message);
    return true;
}

std::optional<MatchInfo> GameRoomManager::GetRoomMatchInfo(RoomId room_id) {
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return std::nullopt;
    }
    return room_opt.value()->CreateMatchInfo();
}

using Result = MsgProto::RoomReqResult;
Result GameRoomManager::CreateRoom(GameRoomInfo&& room_info, std::string& out_room_code) {
    
    RoomId new_room_id = next_room_id_++;
    std::string generated_code = RoomUtils::GenRoomCode();
    room_info.room_id = new_room_id;
    room_info.room_code = generated_code;
    auto new_room = std::make_shared<GameRoom>(std::move(room_info), executor_);
    new_room->SetMessageGateway(message_gateway_);

    auto player_manager = player_manager_.lock();
    if (!player_manager) {
        return Result::UNKNOWN_ERROR;
    }
    auto owner_player = player_manager->GetPlayer(room_info.owner_id);
    if (!owner_player) {
        return Result::NOT_AUTHORIZED;
    }

    Result join_owner_result = new_room->JoinRoom(owner_player);
    if (join_owner_result != Result::OK) {
        return join_owner_result;
    }
    out_room_code = generated_code;

    std::unique_lock lock(mutex_);
    room_code_map_.insert({generated_code, new_room_id});
    rooms_[new_room_id] = new_room;
    return Result::OK;
}

Result GameRoomManager::JoinRoom(RoomId room_id, UserId player_id){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }

    auto player_manager = player_manager_.lock();
    if (!player_manager) {
        return Result::UNKNOWN_ERROR;
    }
    auto player = player_manager->GetPlayer(player_id);
    if (!player) {
        return Result::NOT_AUTHORIZED;
    }
    return room_opt.value()->JoinRoom(player);
}

Result GameRoomManager::LeaveRoom(RoomId room_id, UserId player_id){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->LeaveRoom(player_id);
}

Result GameRoomManager::SetReady(RoomId room_id, UserId player_id, bool ready){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->SetReady(player_id, ready);
}

Result GameRoomManager::StartGame(RoomId room_id, UserId player_id){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->StartGame(player_id);
}

Result GameRoomManager::ChangeMap(RoomId room_id, UserId player_id, const std::string& map_path){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->ChangeMap(player_id, map_path);
}

Result GameRoomManager::ChangeRoomName(RoomId room_id, UserId player_id, const std::string& new_name){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->ChangeRoomName(player_id, new_name);
}

Result GameRoomManager::ChangePassword(RoomId room_id, UserId player_id, const std::string& new_password){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->ChangePassword(player_id, new_password);
}

Result GameRoomManager::KickPlayer(RoomId room_id, UserId player_id, UserId target_id){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->KickPlayer(player_id, target_id);
}

Result GameRoomManager::ChangeCapacity(RoomId room_id, UserId player_id, size_t new_capacity){
    auto room_opt = GetRoom(room_id);
    if (!room_opt) {
        return Result::NOT_FOUND;
    }
    return room_opt.value()->ChangeCapacity(player_id, new_capacity);
}

Result GameRoomManager::DissolveRoom(RoomId room_id, UserId player_id){
    std::unique_lock lock(mutex_);
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        return Result::NOT_FOUND;
    }
    auto room = it->second;
    Result result = room->DissolveRoom(player_id);
    if(result != Result::OK) {
        return result;
    }
    room_code_map_.erase(room->GetRoomCode());
    rooms_.erase(it);
    return Result::OK;
}
}

