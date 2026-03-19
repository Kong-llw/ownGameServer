#pragma once

#include <vector>
#include <memory>
#include <string>
#include "GamePlayerInfo.hpp"

namespace Network {
    class IMessageSender; // 前置声明
}

namespace Game {
    class GamePlayer {
    public:
        GamePlayer() = delete;
        explicit GamePlayer(UserId user_id)
            : info_{UserBaseInfo{user_id, 0, std::string("player_") + std::to_string(user_id), 0, false}, 0, 0, 0, false},
            message_sender_(std::weak_ptr<Network::IMessageSender>()) {}

        //~GamePlayer() = default;

        UserId GetPlayerId() const { return info_.b_info.user_id; }
        GroupId GetGroupId() const { return info_.b_info.current_group_id; }
        const GamePlayerInfo& GetInfo() const { return info_; }
        void UpdateInfo(const GamePlayerInfo& new_info) { info_ = new_info; }

        void SetSession(SessionId session_id) { info_.b_info.session_id = session_id; }
        void SetOnline(bool online) { info_.b_info.is_online = online; }
        void SetUserName(const std::string& name) { info_.b_info.user_name = name; }
        void SetReady(bool ready) { info_.ready = ready; }

        void EnterRoom(RoomId room_id){info_.b_info.current_group_id = room_id;};
        void LeaveRoom(){info_.b_info.current_group_id = 0;};
        void SetMessageSender(std::shared_ptr<Network::IMessageSender> sender) { message_sender_ = sender; }
        // 发送消息接口
        bool SendMessage(const std::vector<std::byte>& message);

    private:
        GamePlayerInfo info_;
        std::weak_ptr<Network::IMessageSender> message_sender_; // 用于发送消息的接口
    };
}
