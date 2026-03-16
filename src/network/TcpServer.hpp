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
private:
    void accept_connections();

    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
public:
    TcpServer(asio::io_context& io_context);
    static constexpr uint16_t kServerPort = 20002;
    
    std::shared_ptr<UserSessionMap> user_session_map_;
    std::shared_ptr<MsgRouter> msg_router_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<IMessageCodec> codec_;
};

} // namespace Network