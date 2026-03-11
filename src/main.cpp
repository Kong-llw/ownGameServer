#include <iostream>
#include <asio.hpp>

int main()
{
    try {
        asio::io_context io_context;
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](std::error_code, int){
            std::cout<< "Signal recevied. Stopping server..." << std::endl;
            io_context.stop();
        });

        TcpServer server(io_context);
        std::cout << "Server is running on port " << SERVER_PORT << std::endl;
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
