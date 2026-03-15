#pragma once
//和业务衔接的通信协议相关的结构体、枚举等
#include <cstdint>
#include <string>
namespace MsgProto {
enum class MsgType : std::uint8_t {
    CHATMSG = 0,
    HEARTBEAT = 1,
    JSONCOMMAND = 2,
    JSONDATA = 3,
    ROOMREQ = 4,
    ROOMRSP = 5,
    LOGINREQ = 6,
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
}
struct UserLoginInfo { //login logout共用
    MsgProto::LoginResult result;
    UserId user_id;//logout时使用
    SessionId session_id; //login时使用
    std::string error_msg;
};

struct RoomJoinInfo {
    MsgProto::RoomReqResult result;
    UserId user_id;
    RoomId room_id;
    std::string error_msg;
};

//DataBase Proto
struct DBLoginReq{
    std::string user_name;
    std::string password;
};

struct DBLoginRsp{
    UserId user_id;
    MsgProto::LoginResult result;
    std::string error_msg; //登录失败时的错误信息
};
