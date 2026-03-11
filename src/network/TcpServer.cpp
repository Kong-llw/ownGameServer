#include <asio.hpp>
#include <iostream>

using tcp = asio::ip::tcp;
#define SERVER_PORT 20002

class TcpServer {
public:
        TcpServer(asio::io_context& io_context) : 
            acceptor_(io_context, tcp::endpoint(tcp::v4(), SERVER_PORT)), 
            io_context_(io_context),
            _roomMgr(RoomManager::instance()),
            _dispatcher(io_context.get_executor(), &_roomMgr, &player_manager_),
            session_manager_(io_context.get_executor(), &_dispatcher) {
                session_manager_.SetOnSessionClose([this](std::shared_ptr<ClientSession> session) {
                    this->player_manager_.LogOut(session);
                });
                accept_connections();
            }

private:
    void accept_connections() {
        acceptor_.async_accept(
            [this](std::error_code ec, tcp::socket socket){
                if (!ec) {
                    if(session_manager_.isConnectionFull()){
                        std::cerr << "Server full! Reject connection: " << socket.remote_endpoint().address().to_string() << std::endl;
                        socket.close();
                    }else{
                        int ssId = session_manager_.CreateSession(std::move(socket));
                        std::cout << "Connect In SessionId:" << ssId << std::endl;
                    }

                }
                else{
                    std::cerr << "Accept error: " << ec.message() << std::endl;
                }

                accept_connections();
            });
    }

    tcp::acceptor acceptor_;
    asio::io_context& io_context_;
    RoomManager& _roomMgr;
    PlayerManager player_manager_;
    MessageDispatcher _dispatcher;
    SessionManager session_manager_;
};