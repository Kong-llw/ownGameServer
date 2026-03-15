#pragma once
//整个Server的初始化入口
#include <asio.hpp>
#include <iostream>
#include <memory>

#include "protocol/Router/UserSessionMap.hpp"
#include "protocol/Router/MsgRouter.hpp"
#include "network/session/SessionManager.hpp"
#include "network/codec/IMessageCodec.hpp"

namespace Network {

class IMessageCodec;
class UserSessionMap;
class MsgRouter;
class SessionManager;

using tcp = asio::ip::tcp;

class TcpServer {
public:
    static constexpr uint16_t kServerPort = 20002;

    TcpServer(asio::io_context& io_context);
    std::shared_ptr<UserSessionMap> user_session_map_;
    std::shared_ptr<MsgRouter> msg_router_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<IMessageCodec> codec_;
private:
    void accept_connections();

    tcp::acceptor acceptor_;
    asio::io_context& io_context_;
};

} // namespace Network