#pragma once

#include <vector>
#include <memory>
#include <string>
#include <span>
#include <cstddef>
#include "core/Types.h"
#include "business/Group/Group.hpp"
#include "business/GameLogic/GameProto.hpp"
#include "network/Router/IBusinessMsgGateway.hpp"
#include "protocol/ProtocolType.hpp"

struct GamePlayerInfo;
enum class RoomState { LOBBY = 0, RUNNING = 1 };

// 房间操作返回码
enum class RoomReqResult {
    OK = 0,
    FULL,
    NOT_FOUND,
    ALREADY_IN_ROOM,
    NOT_IN_ROOM,                                    
    NOT_OWNER,
    INVALID_CAPACITY,
    ALREADY_RUNNING,
    NOT_READY,
    GEN_ROOMCODE_FAILED,

    EMPTY_REQ,
    NOT_AUTHORIZED,
    UNKNOWN_ERROR
};

struct RoomInListInfo{      //客户端搜寻房间的时候，列表显示需要的信息
    std::string room_code;  //客户端识别房间用的 大写字母+数字
    std::string room_name;  //房间名称
    size_t capacity;        //房间容量
    size_t player_count;    //当前玩家数量
    RoomState state;        //房间状态
};

struct GameRoomInfo {
    RoomId room_id;
    std::string room_code;
    std::string room_name;
    std::string password;
    UserId owner_id;
    size_t capacity;        //房间容量
    size_t player_count;    //当前玩家数量
    RoomState state;        //房间状态
    std::string selected_map_path; 
};

namespace Game {
    class GameRoom {
    public:
        GameRoom();
        ~GameRoom();
        GameRoomInfo GetAllInfo() const {
            auto snapshot = info_;
            snapshot.capacity = group_.GetCapacity();
            snapshot.player_count = group_.GetMemberCount();
            return snapshot;
        }
        RoomInListInfo GetInListInfo() const {
            return RoomInListInfo{
                info_.room_code,
                info_.room_name,
                group_.GetCapacity(),
                group_.GetMemberCount(),
                info_.state,
            };
        }
        const std::vector<UserId>& GetPlayersId() const { return group_.GetMembers(); }

        MatchInfo CreateMatchInfo(); // 游戏开始时根据创建房间信息，用于创建状态机
        RoomReqResult JoinRoom(UserId player_id) { return MapGroupResult(group_.AddMember(player_id)); }
        RoomReqResult LeaveRoom(UserId player_id) { return MapGroupResult(group_.RemoveMember(player_id)); }
        RoomReqResult SetReady(UserId player_id, bool ready);
        RoomReqResult StartGame(UserId player_id);
        RoomReqResult ChangeMap(UserId player_id, const std::string& map_path);
        RoomReqResult ChangeRoomName(UserId player_id, const std::string& new_name) {
            if (player_id != info_.owner_id) {
                return RoomReqResult::NOT_OWNER;
            }
            info_.room_name = new_name;
            return RoomReqResult::OK;
        }
        RoomReqResult ChangePassword(UserId player_id, const std::string& new_password) {
            if (player_id != info_.owner_id) {
                return RoomReqResult::NOT_OWNER;
            }
            info_.password = new_password;
            return RoomReqResult::OK;
        }
        RoomReqResult KickPlayer(UserId player_id, UserId target_id);
        RoomReqResult ChangeCapacity(UserId player_id, size_t new_capacity) {
            if (player_id != info_.owner_id) {
                return RoomReqResult::NOT_OWNER;
            }
            return MapGroupResult(group_.SetCapacity(new_capacity));
        }
        RoomReqResult DissolveRoom(UserId player_id);

        void Broadcast(std::span<const std::byte> message, ProtoInfo::ProtocolType ptype);
        void SendTo(UserId playerId, std::span<const std::byte> message, ProtoInfo::ProtocolType ptype);

        void SetMessageGateway(std::shared_ptr<Network::IBusinessMsgGateway> gateway) { message_gateway_ = gateway; }
    private:
        static RoomReqResult MapGroupResult(GroupResult result) {
            switch (result) {
            case GroupResult::OK:
                return RoomReqResult::OK;
            case GroupResult::FULL:
                return RoomReqResult::FULL;
            case GroupResult::ALREADY_IN_GROUP:
                return RoomReqResult::ALREADY_IN_ROOM;
            case GroupResult::NOT_IN_GROUP:
                return RoomReqResult::NOT_IN_ROOM;
            case GroupResult::INVALID_CAPACITY:
                return RoomReqResult::INVALID_CAPACITY;
            }
            return RoomReqResult::UNKNOWN_ERROR;
        }

        GameRoomInfo info_;
        Group group_;
        std::shared_ptr<Network::IBusinessMsgGateway> message_gateway_;
    };
}
