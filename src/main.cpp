#include <iostream>
#include <thread>

#include <asio.hpp>

#include "network/TcpServer.hpp"
#include "business/ReqHandler.hpp"
#include "business/HeartbeatHandler.hpp"
#include "business/GameRoom/GameRoomManager.hpp"
#include "business/GamePlayer/GamePlayerManager.hpp"
#include "core/User/UserStateManager.hpp"

struct InitResources {
    std::shared_ptr<ReqHandler> req_handler;
    std::shared_ptr<HearbeatHandler> heartbeat_handler;
    std::shared_ptr<Game::GameRoomManager> room_manager;
    std::shared_ptr<UserStateManager> user_state_manager;
    std::shared_ptr<Game::GamePlayerManager> player_manager;
};

void initialize(Network::TcpServer& server, asio::io_context& business_context, InitResources& resources) {
    asio::any_io_executor business_executor = business_context.get_executor();
    
    resources.req_handler = std::make_shared<ReqHandler>(asio::make_strand(business_executor));
    resources.req_handler->SetGateway(server.msg_router_);
    resources.heartbeat_handler = std::make_shared<HearbeatHandler>();
    resources.heartbeat_handler->SetGateway(server.msg_router_);
    resources.user_state_manager = std::make_shared<UserStateManager>();
    resources.player_manager = std::make_shared<Game::GamePlayerManager>(resources.user_state_manager);
    resources.room_manager = std::make_shared<Game::GameRoomManager>(business_executor);
    resources.room_manager->SetPlayerManager(resources.player_manager);
    
    server.msg_router_->SetRoomManager(resources.room_manager);
    server.msg_router_->SetSessionManager(server.session_manager_);
    /*注册消息处理器， 目前这个是并不符合最初预期的， 预定是要根据 ProtoType去分发，但是现在基本只有业务处理器，
    只有心跳包不是业务处理逻辑， 所以就把心跳也作为一个MsgType，用MsgType去映射了*/
    server.msg_router_->RegisterMsgHandler(static_cast<uint8_t>(MsgProto::MsgType::HEARTBEAT), resources.heartbeat_handler);
    server.msg_router_->RegisterMsgHandler(static_cast<uint8_t>(MsgProto::MsgType::LOGINREQ), resources.req_handler);
    server.msg_router_->RegisterMsgHandler(static_cast<uint8_t>(MsgProto::MsgType::ROOMREQ), resources.req_handler);
    //chatmsg->

    resources.heartbeat_handler->RegisterHeartbeatCallback([session_manager = server.session_manager_](SessionId session_id){
        session_manager->onSessionHeartbeat(session_id);
    });
    resources.req_handler->RegLoginCallback([user_map = server.user_session_map_](UserLoginInfo info) {
        user_map->onUserLogin(info);
    });
    resources.req_handler->RegLoginCallback([player_manager = resources.player_manager](UserLoginInfo info) {
        player_manager->onUserLogin(info);
    });
    resources.req_handler->RegLogoutCallback([user_map = server.user_session_map_](UserLoginInfo info) {
        user_map->onUserLogout(info);
    });
    resources.req_handler->RegLogoutCallback([player_manager = resources.player_manager](UserLoginInfo info) {
        player_manager->onUserLogout(info);
    });
    //其他业务handler的注册
}

int main()
{
    constexpr int kThreadNum = 4-1;
    try {
        asio::io_context io_context;

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](std::error_code, int){
            std::cout<< "Signal recevied. Stopping Net..." << std::endl;
            io_context.stop();
        });

        Network::TcpServer server(io_context);

        InitResources resources;//持有一些需要用的非网络相关的资源
        initialize(server, io_context, resources); //注入依赖
        for(int i = 0;i < kThreadNum; ++i) {
            std::thread([&io_context](){
                io_context.run();
            }).detach();
        }

        std::cout << "Server is running on port " << Network::TcpServer::kServerPort << std::endl;
        io_context.run();
        std::cout << "Server stopped (io_context exited)" << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Main Exception: " << e.what() << std::endl;
        return 1;
    }

    //LOG_INFO("Server stoped normally."); 
    return 0;
}

