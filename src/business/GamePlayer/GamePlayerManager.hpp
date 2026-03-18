#pragma once
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <optional>
#include <string>
#include "business/GamePlayer/GamePlayer.hpp"
#include "core/User/UserStateManager.hpp"
#include "network/session/IMessageSender.hpp"

namespace Game {
    class GamePlayerManager {
    public:
        GamePlayerManager(std::shared_ptr<UserStateManager> user_state_manager) 
        : user_state_manager_(std::move(user_state_manager)) {}
        ~GamePlayerManager() = default;

        // 创建玩家实例
        std::shared_ptr<GamePlayer> CreatePlayer(UserId player_id);
        // 获取玩家实例
        std::shared_ptr<GamePlayer> GetPlayer(UserId player_id);
        //获取游戏中需要的信息
        std::optional<GamePlayerInfo> GetPlayerInfo(UserId player_id) const;
        // 移除玩家实例
        void RemovePlayer(UserId player_id);
        void onUserLogin(UserLoginInfo info);
        void onUserLogout(UserLoginInfo info);
        void SetPlayerRoom(UserId player_id, RoomId room_id);
    private:
        std::unordered_map<UserId, std::shared_ptr<GamePlayer>> players_;
        std::shared_ptr<UserStateManager> user_state_manager_; // 引用用户状态管理器 不含业务信息
        mutable std::shared_mutex player_manager_mutex_; // 保护players_的线程安全
    };  
}