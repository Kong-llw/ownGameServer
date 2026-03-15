//登陆退出 进出房间之类的消息处理  二次转发到具体执行类

#pragma once
#include <vector>
#include <asio.hpp>
#include "protocol/MessageProto.hpp"
#include "network/codec/IMessageCodec.hpp"
#include "business/IMsgHandler.hpp"
#include "protocol/Router/IBusinessMsgGateway.hpp"

class ReqHandler : public IMsgHandler {
public:
    explicit ReqHandler(asio::strand<asio::any_io_executor> strand) : strand_(std::move(strand)) {};
    using LoginCallback = std::function<void(UserLoginInfo)>;
    using LogoutCallback = std::function<void(UserLoginInfo)>;
    using RoomJoinCallback = std::function<void(RoomJoinInfo)>;
    using RoomLeaveCallback = std::function<void(RoomJoinInfo)>;

    bool HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& msg) override;
    void SetGateway(std::shared_ptr<Network::IBusinessMsgGateway> gateway) { gateway_ = std::move(gateway); }
    
    bool RegLoginCallback(LoginCallback cb) {
        login_callbacks_.push_back(std::move(cb));
        return true;
    }
    bool RegRoomJoinCallback(RoomJoinCallback cb) {
        join_room_callbacks_.push_back(std::move(cb));
        return true;
    }
    bool RegLogoutCallback(LogoutCallback cb) {
        logout_callbacks_.push_back(std::move(cb));
        return true;
    }
    bool RegRoomLeaveCallback(RoomLeaveCallback cb) {
        leave_room_callbacks_.push_back(std::move(cb));
        return true;
    }
private:
    asio::strand<asio::any_io_executor> strand_; //保证同一时间只有一个请求在处理, 避免并发问题
    std::shared_ptr<Network::IBusinessMsgGateway> gateway_; //用于转发消息

    bool HandleLoginRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    bool HandleLogoutRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    bool HandleRoomJoinRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    bool HandleRoomLeaveRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    DBLoginRsp ValidateLogin(const DBLoginReq& req);
    std::vector<LoginCallback> login_callbacks_;
    std::vector<LogoutCallback> logout_callbacks_;
    std::vector<RoomJoinCallback> join_room_callbacks_;
    std::vector<RoomLeaveCallback> leave_room_callbacks_;

    void SendBackLoginResponse(const std::shared_ptr<Network::MsgPack>& msg, const DBLoginRsp& rsp);
};