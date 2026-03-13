#pragma once

#include "network/codec/IMessageCodec.hpp"

class IMsgHandler {
public:
    virtual ~IMsgHandler() = default;
    virtual bool HandleDecodedMsg(Network::DecodedMessage& msg) = 0;
};