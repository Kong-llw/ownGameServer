#include "TcpServer.hpp"

#include <utility>

#include "network/codec/mTcpCodec.hpp"
#include "network/session/SessionManager.hpp"
#include "protocol/Router/MsgRouter.hpp"
#include "protocol/Router/UserSessionMap.hpp"

namespace Network {

TcpServer::TcpServer(asio::io_context& io_context)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), kServerPort)),
      io_context_(io_context),
      user_session_map_(std::make_shared<UserSessionMap>()),
      msg_router_(std::make_shared<MsgRouter>(user_session_map_)),
      session_manager_(std::make_shared<SessionManager>(
          io_context.get_executor(), user_session_map_, std::static_pointer_cast<IBusinessMsgGateway>(msg_router_))),
      codec_(mTcpCodec::Shared()) {
    session_manager_->SetOnSessionClose([](std::shared_ptr<ClientSession> /*session*/) {
        // Hook: logout/cleanup can be bound here later.
    });
    accept_connections();
}

void TcpServer::accept_connections() {
    acceptor_.async_accept([this](std::error_code ec, tcp::socket socket) {
        if (!ec) {
            const SessionId created = session_manager_->CreateSession(std::move(socket), codec_);
            if (created == SessionId{}) {
                std::cerr << "CreateSession failed for accepted socket" << std::endl;
            } else {
                std::cout << "New connection accepted. Session ID: " << created << std::endl;
            }
        } else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
        }
        accept_connections();
    });
}

} // namespace Network