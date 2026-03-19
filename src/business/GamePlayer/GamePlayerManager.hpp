#pragma once
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <optional>
#include <string>
#include "business/GamePlayer/GamePlayer.hpp"
#include "network/session/IMessageSender.hpp"
#include "protocol/MessageProto.hpp"
namespace Game {
    class GamePlayerManager {
    public:
        GamePlayerManager() = default;
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
        // 玩家自身管理用户数据，PlayerManager 只负责管理 Player 实例
        mutable std::shared_mutex player_manager_mutex_; // 保护players_的线程安全
    };  
}