#include "GameRoom.hpp"

namespace Game{
using Result = MsgProto::RoomReqResult;
GameRoomInfo GameRoom::GetAllInfo() const {
    auto snapshot = info_;
    snapshot.capacity = group_.GetCapacity();
    snapshot.player_count = group_.GetMemberCount();
    auto vec = group_.GetMembers();
    std::vector<GamePlayerInfo> player_infos;
    for (const auto& player : vec) {
        if (player) {
            player_infos.push_back(player->GetInfo());
        }
    }
    snapshot.players = std::move(player_infos);
    return snapshot;
}

RoomInListInfo GameRoom::GetInListInfo() const {
    return RoomInListInfo{
        info_.room_code,
        info_.room_name,
        group_.GetCapacity(),
        group_.GetMemberCount(),
        info_.state,
    };
}

std::vector<UserId> GameRoom::GetPlayersId() const {
    std::vector<UserId> player_ids;
    player_ids.reserve(group_.GetMemberCount());
    for (const auto& player : group_.GetMembers()) {
        if (player) player_ids.push_back(player->GetPlayerId());
    }
    return player_ids;
}

MatchInfo GameRoom::CreateMatchInfo() { //MARK 修改一下，加入房间的时候需要这个来渲染客户端
    std::vector<GamePlayerInfo> player_infos;
    player_infos.reserve(group_.GetMemberCount());
    for (const auto& player : group_.GetMembers()) {
        if (player) player_infos.push_back(player->GetInfo());
    }
    MatchInfo match_info{
        std::move(player_infos),
        info_.selected_map_path,
        info_.owner_id
    };
    return match_info;
}

Result GameRoom::JoinRoom(std::shared_ptr<GamePlayer> player) {
    if (IsRunning()) {
        return Result::GAME_RUNNING;
    }
    if (!player || player->GetPlayerId() == UserId{}) {
        return Result::NOT_AUTHORIZED;
    }
    player->EnterRoom(info_.room_id);
    Result ret = MapGroupResult(group_.AddMember(std::move(player)));
    if(ret == Result::OK){
        info_.player_count++;
    }
    return ret;
}

Result GameRoom::LeaveRoom(UserId player_id) {
    Result ret = MapGroupResult(group_.RemoveMember(player_id));
    if(ret == Result::OK){
        info_.player_count--;
    }
    return ret;
}

Result GameRoom::SetReady(UserId player_id, bool ready) {
    auto* player_ptr = group_.FindMember(player_id);
    if (player_ptr == nullptr) {
        return Result::NOT_IN_ROOM;
    }
    auto& sp = *player_ptr; // sp is std::shared_ptr<GamePlayer>
    if (!sp) return Result::NOT_IN_ROOM;
    sp->SetReady(ready);
    return Result::OK;
}

Result GameRoom::StartGame(UserId player_id) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    if (IsRunning()) {
        return Result::ALREADY_RUNNING;
    }
    if (group_.GetMemberCount() < 2) { // 最少需要2人才能开始游戏
        return Result::ILLEGAL_OPERATION;
    }

    if (!cmd_handler_) {
        return Result::UNKNOWN_ERROR;
    }

    cmd_handler_->InitGameState();
    cmd_handler_->StartGame(CreateMatchInfo());
    cmd_handler_->StartTicking();

    SetRoomState(RoomState::RUNNING);
    return Result::OK;
}

Result GameRoom::ChangeMap(UserId player_id, const std::string& map_path) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    if (info_.state != RoomState::LOBBY) {
        return Result::ALREADY_RUNNING;
    }
    info_.selected_map_path = map_path;
    return Result::OK;
}

Result GameRoom::ChangeRoomName(UserId player_id, const std::string& new_name) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    info_.room_name = new_name;
    return Result::OK;
}

Result GameRoom::ChangePassword(UserId player_id, const std::string& new_password) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    info_.password = new_password;
    return Result::OK;
}

Result GameRoom::KickPlayer(UserId player_id, UserId target_id) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    if (IsRunning()) {
        return Result::GAME_RUNNING;
    }
    if (target_id == player_id) {
        return Result::ILLEGAL_OPERATION;
    }
    return MapGroupResult(group_.RemoveMember(target_id));
}

Result GameRoom::ChangeCapacity(UserId player_id, size_t new_capacity) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    return MapGroupResult(group_.SetCapacity(new_capacity));
}

Result GameRoom::DissolveRoom(UserId player_id) {
    if (player_id != info_.owner_id) {
        return Result::NOT_OWNER;
    }
    // 这里的实现可能需要通知所有玩家房间被解散了，具体逻辑可以根据实际需求调整
    if (cmd_handler_) {
        cmd_handler_->StopTicking();
    }
    group_.ClearMembers();
    SetRoomState(RoomState::LOBBY);
    return Result::OK;
}

void GameRoom::Broadcast(Network::EncodeMessage& message) {
    for (const auto& player : group_.GetMembers()) {
        if (player) SendTo(player->GetPlayerId(), message);
    }
}
void GameRoom::OnGameEnd(const UserId winner_id) {
    // 游戏结束时的逻辑处理，例如通知所有玩家游戏结束了，宣布赢家等
    // 具体实现可以根据实际需求调整
    SetRoomState(RoomState::LOBBY);
    if (cmd_handler_) {
        cmd_handler_->StopTicking();
    }
    Network::EncodeMessage msg{
            .msg_id = 0,
            .proto_type = 0, // 根据实际协议类型设置
            .main_type = static_cast<uint8_t>(MsgProto::MsgType::STATECONTROL),
            .sub_type = static_cast<uint8_t>(MsgProto::FSMState::END),
            .payload = std::span<const std::byte>(
                reinterpret_cast<const std::byte*>(&winner_id),
                sizeof(winner_id)
            ),
            .payload_owner = nullptr, // 如果需要，可以设置为一个拥有payload生命周期的对象
        };
    message_gateway_->BroadcastToRoom(info_.room_id, msg);
}

void GameRoom::SendTo(UserId player_id, Network::EncodeMessage& message) {
    message_gateway_->SendMessageToUser(player_id, message);
}
}