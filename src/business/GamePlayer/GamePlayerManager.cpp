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
    //要求用户合法
    std::optional<UserBaseInfo> user_base_info = user_state_manager_->GetUserState(player_id);
    if (!user_base_info) {
        return nullptr;
    }
    //根据用户信息构造玩家信息
    auto player = std::make_shared<GamePlayer>(user_base_info.value());
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
    auto user_state = user_state_manager_->GetUserState(player_id);
    if (!user_state) {
        return std::nullopt;
    }

    GamePlayerInfo info{
        *user_state,
        0,
        0,
        0,
        false,
    };
    return info;
}
// 移除玩家实例
void GamePlayerManager::RemovePlayer(UserId player_id) {
    std::unique_lock lock(player_manager_mutex_);
    players_.erase(player_id);
}

void GamePlayerManager::onUserLogin(UserLoginInfo info) {
    if(info.result != MsgProto::LoginResult::SUCCESS|| info.user_id == UserId{}) {
        return;
    }
    user_state_manager_->OnUserLogin(info);
    CreatePlayer(info.user_id);
}

void GamePlayerManager::onUserLogout(UserLoginInfo info) {
    if(info.user_id == UserId{}) {
        return;
    }
    user_state_manager_->OnUserLogout(info);
    RemovePlayer(info.user_id);
}

void GamePlayerManager::SetPlayerRoom(UserId player_id, RoomId room_id){
    std::shared_ptr<GamePlayer> player = GetPlayer(player_id);
    if(player) {
        player->EnterRoom(room_id);
    }
}
}