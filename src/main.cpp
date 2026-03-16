#include <iostream>
#include <thread>

#include <asio.hpp>

#include "network/TcpServer.hpp"
#include "business/ReqHandler.hpp"
#include "business/HeartbeatHandler.hpp"

struct InitResources {
    std::shared_ptr<ReqHandler> req_handler;
    std::shared_ptr<HearbeatHandler> heartbeat_handler;
};

void initialize(Network::TcpServer& server, asio::io_context& business_context, InitResources& resources) {
    asio::any_io_executor business_executor = business_context.get_executor();
    resources.req_handler = std::make_shared<ReqHandler>(asio::make_strand(business_executor));
    resources.req_handler->SetGateway(server.msg_router_);
    resources.heartbeat_handler = std::make_shared<HearbeatHandler>();
    resources.heartbeat_handler->SetGateway(server.msg_router_);

    /*注册消息处理器， 目前这个是并不符合最初预期的， 预定是要根据 ProtoType去分发，但是现在基本只有业务处理器，
    只有心跳包不是业务处理逻辑， 所以就把心跳也作为一个MsgType，用MsgType去映射了*/
    server.msg_router_->RegisterMsgHandler(static_cast<uint8_t>(MsgProto::MsgType::HEARTBEAT), resources.heartbeat_handler);
    server.msg_router_->RegisterMsgHandler(static_cast<uint8_t>(MsgProto::MsgType::LOGINREQ), resources.req_handler);
    server.msg_router_->RegisterMsgHandler(static_cast<uint8_t>(MsgProto::MsgType::ROOMREQ), resources.req_handler);

    server.msg_router_->SetSessionManager(server.session_manager_);

    resources.heartbeat_handler->RegisterHeartbeatCallback([session_manager = server.session_manager_](SessionId session_id){
        session_manager->onSessionHeartbeat(session_id);
    });
    resources.req_handler->RegLoginCallback([user_map = server.user_session_map_](UserLoginInfo info) {
        user_map->onUserLogin(std::move(info));
    });
    resources.req_handler->RegLogoutCallback([user_map = server.user_session_map_](UserLoginInfo info) {
        user_map->onUserLogout(std::move(info));
    });
    //其他业务handler的注册
}

int main()
{
    try {
        asio::io_context net_context;
        asio::io_context business_context;

        asio::signal_set signals(net_context, SIGINT, SIGTERM);
        signals.async_wait([&](std::error_code, int){
            std::cout<< "Signal recevied. Stopping Net..." << std::endl;
            net_context.stop();
            business_context.stop();
        });

        Network::TcpServer server(net_context);

        InitResources resources;
        initialize(server, business_context, resources); //注入依赖
        std::jthread business_thread([&]() {
            business_context.run();
        });

        std::cout << "Server is running on port " << Network::TcpServer::kServerPort << std::endl;
        net_context.run();
        business_context.stop();
        std::cout << "Server stopped (io_context exited)" << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Main Exception: " << e.what() << std::endl;
        return 1;
    }

    //LOG_INFO("Server stoped normally."); 
    return 0;
}

