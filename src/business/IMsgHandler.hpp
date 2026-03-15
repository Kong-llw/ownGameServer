#pragma once

#include "network/codec/IMessageCodec.hpp"

class IMsgHandler {
public:
    virtual ~IMsgHandler() = default;
    //此函数必须为异步实现, 避免阻塞Router
    virtual bool HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& msg) = 0;
    virtual bool Authentication(const std::shared_ptr<Network::MsgPack>& msg) { return true; } //默认不鉴权
};