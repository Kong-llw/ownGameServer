#pragma once

#include <cstdint>

namespace ProtoInfo {
enum class ProtocolType : std::uint8_t {
    CHATMSG = 0,
    HEARTBEAT = 1,
    JSONCOMMAND = 2,
    JSONDATA = 3,
    ROOMREQ = 4,
    ROOMRSP = 5,
    LOGIN = 6,
    STATECONTROL = 7,
    CMDREQ = 8,
    CMDRSP = 9,
};

enum class RoomReqType : uint8_t {
    CREATEROOM = 1,
    JOINROOM = 2,
    LEAVEROOM = 3,
    LISTROOMS = 4,
    SETREADY = 5,
    SETCAPACITY = 6,
    STARTGAME = 7,
    DISSOLVEROOM = 8,
    INITCOMPLETE = 9,
};

enum class LoginResult : uint8_t {
    SUCCESS = 0,
    INVALID_CREDENTIALS = 1,
    USER_BANNED = 2,
    SERVER_FULL = 3,
    ALREADY_ONLINE = 4,
    UNKNOWN = 5,
};

enum class StartGameResult : uint8_t {
    GAME_START_SUCCESS = 0,
    NOT_ENOUGH_PLAYERS = 1,
    PLAYERS_NOT_READY = 2,
    INSUFFICIENT_CAPACITY = 3,
    UNKNOWN_ERROR = 4,
};

enum class FSMState : uint8_t {
    Init,       //导入数据
    MapLoad,    //配置地图
    WaitForPlayers, //就绪,等待其他玩家
    BattleIntroAnimation, // 入场动画
    
    //战斗循环
    TurnStart, //重置数据 显示效果等 触发结算之类的
    TurnWaitInput, //接收输入
    TurnActionResolving, //
    EXPAND,
    GROW,
    TurnEndProcessing,
    BattleCheckResult,
    END,
    BattleCleanup,
};
}

//DataBase Proto
struct DBLoginRsp{
    ProtoInfo::LoginResult result;
    uint32_t MsgLen;
    uint64_t user_id;
};
