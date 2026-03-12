#pragma once

#include <vector>
#include <memory>
#include "GamePlayerInfo.hpp"

namespace Network {
    class IMessageSender; // 前置声明
}

namespace Game {
    class GamePlayer {
    public:
        GamePlayer(UserId player_id, const std::string& name)
            : info_{UserBaseInfo{player_id, 0, name, 0, true}, 0, 0, 0, false},
            message_sender_(std::weak_ptr<Network::IMessageSender>()) {}
        ~GamePlayer();

        UserId GetPlayerId() const { return info_.b_info.user_id; }
        GroupId GetGroupId() const { return info_.b_info.current_group_id; }
        RoomId GetRoomId() const { return GetGroupId(); }
        const GamePlayerInfo& GetInfo() const { return info_; }
        void UpdateInfo(const GamePlayerInfo& new_info);

        void EnterRoom(RoomId room_id);
        void LeaveRoom();
        void SetMessageSender(std::shared_ptr<Network::IMessageSender> sender) { message_sender_ = sender; }
        // 发送消息接口
        bool SendMessage(const std::vector<std::byte>& message);

    private:
        GamePlayerInfo info_;
        std::weak_ptr<Network::IMessageSender> message_sender_; // 用于发送消息的接口
    };
}
