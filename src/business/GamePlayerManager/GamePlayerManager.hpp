#pragma once
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include "business/GamePlayer/GamePlayer.hpp"
#include "core/User/UserStateManager.hpp"
#include "network/session/IMessageSender.hpp"

namespace Game {
    class GamePlayerManager {
    public:
        GamePlayerManager(UserStateManager& user_state_manager) : user_state_manager_(user_state_manager) {}
        ~GamePlayerManager();

        // 创建玩家实例
        std::shared_ptr<GamePlayer> CreatePlayer(UserId player_id, const std::string& name);
        // 获取玩家实例
        std::shared_ptr<GamePlayer> GetPlayer(UserId player_id);
        // 移除玩家实例
        void RemovePlayer(UserId player_id);

    private:
        std::unordered_map<UserId, std::shared_ptr<GamePlayer>> players_;
        UserStateManager& user_state_manager_; // 引用用户状态管理器 不含业务信息
        std::shared_mutex player_manager_mutex_; // 保护players_的线程安全
    };  
}