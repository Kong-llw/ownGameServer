#pragma once
#include <functional>
#include <vector>

#include "business/IMsgHandler.hpp"
#include "protocol/MessageProto.hpp"
#include "protocol/Router/IBusinessMsgGateway.hpp"
#include "protocol/mTcpProto.h"
#include "network/codec/IMessageCodec.hpp"
#include "core/Types.h"
class HearbeatHandler : public IMsgHandler {
public:
    bool HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& pack) override {
        if (!pack || !gateway_) {
            return false;
        }

        for (const auto& cb : heartbeat_callbacks_) {
            if (cb) {
                cb(pack->sender_session_id);
            }
        }

        Network::EncodeMessage response_msg{
            pack->msg.msg_id,
            static_cast<uint8_t>(ProtoType::HeartBeat),
            0,
            0,
            {},
            nullptr};

        return gateway_->SendMessageToSession(pack->sender_session_id, response_msg);
    }
    bool Authentication(const std::shared_ptr<Network::MsgPack>& msg) override {
        (void)msg;
        return true; //心跳包不鉴权
    }
    void SetGateway(std::shared_ptr<Network::IBusinessMsgGateway> gateway) { gateway_ = std::move(gateway); }
    void RegisterHeartbeatCallback(std::function<void(SessionId session_id)> cb) {
        heartbeat_callbacks_.push_back(std::move(cb));
    }
private:
    std::shared_ptr<Network::IBusinessMsgGateway> gateway_; //用于转发消息
    std::vector<std::function<void(SessionId session_id)>> heartbeat_callbacks_;
};