#include "GamePlayerManager.hpp"
#include "protocol/MessageProto.hpp"
namespace Game {
std::shared_ptr<GamePlayer> GamePlayerManager::CreatePlayer(UserId player_id) {
    std::unique_lock lock(player_manager_mutex_);
    //请求重复的
    auto it = players_.find(player_id);
    if (it != players_.end()) {
        return it->second;
    }
    // 根据 player_id 构造玩家，由 Player 自行维护数据
    auto player = std::make_shared<GamePlayer>(player_id);
    players_.emplace(player_id, player);
    return player;
}
// 获取玩家实例
std::shared_ptr<GamePlayer> GamePlayerManager::GetPlayer(UserId player_id) {
    std::shared_lock lock(player_manager_mutex_);
    auto it = players_.find(player_id);
    if (it == players_.end()) {
        return nullptr;
    }
    return it->second;
}
std::optional<GamePlayerInfo> GamePlayerManager::GetPlayerInfo(UserId player_id) const {
    std::shared_lock lock(player_manager_mutex_);
    auto it = players_.find(player_id);
    if (it == players_.end()) return std::nullopt;
    return it->second->GetInfo();
}
// 移除玩家实例
void GamePlayerManager::RemovePlayer(UserId player_id) {
    std::unique_lock lock(player_manager_mutex_);
    players_.erase(player_id);
}

void GamePlayerManager::onUserLogin(UserLoginInfo info) {
    if(info.result != MsgProto::LoginResult::SUCCESS || info.user_id == UserId{}) {
        return;
    }
    // 创建玩家并设置会话/在线状态，由 Player 自行维护数据
    auto player = CreatePlayer(info.user_id);
    if (player) {
        player->SetSession(info.session_id);
        player->SetOnline(true);
    }
}

void GamePlayerManager::onUserLogout(UserLoginInfo info) {
    if(info.user_id == UserId{}) {
        return;
    }
    auto player = GetPlayer(info.user_id);
    if (player) {
        player->SetOnline(false);
    }
    RemovePlayer(info.user_id);
}

void GamePlayerManager::SetPlayerRoom(UserId player_id, RoomId room_id){
    std::shared_ptr<GamePlayer> player = GetPlayer(player_id);
    if(player) {
        player->EnterRoom(room_id);
    }
}
}