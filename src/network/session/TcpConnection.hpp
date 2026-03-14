#pragma once
//对Asio::tcp的封装结构，提供对socket的操作 由每个Client Session类独立持有
#include <asio.hpp>
#include <array>
#include <cstddef>
#include <span>
#include <memory>
#include <queue>
#include <functional>
#include <vector>
#include "core/Types.h"
using tcp = asio::ip::tcp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using CloseCallback = std::function<void()>;
    using MessageCallback = std::function<void(std::vector<std::byte>)>;
    
    explicit TcpConnection(tcp::socket socket, SessionId session_id)
        : socket_(std::move(socket)), session_id_(session_id), 
        strand_(asio::make_strand(socket_.get_executor())) {}

    void Start();
    void Send(const std::vector<std::byte>& data);
    void Send(std::vector<std::byte>&& data);
    void Close();

    template<typename Callable> requires std::invocable<Callable>
    void PostToStrand(Callable&& task) {
        asio::post(strand_, std::forward<Callable>(task));
    }
    void SetCloseCallback(CloseCallback callback) { close_callback_ = std::move(callback); }
    void SetMessageCallback(MessageCallback callback) { message_callback_ = std::move(callback); }
    const SessionId GetSessionId() const { return session_id_; }
private:
    void DoRead();
    void DoWrite();
    void DoClose(bool notify_close_cb);

    tcp::socket socket_;
    SessionId session_id_;
    asio::strand<asio::any_io_executor> strand_;
    std::array<std::byte, 4096> read_buffer_;
    std::queue<std::vector<std::byte>> writeQ_;
    bool started_{false};
    bool closed_{false};

    CloseCallback close_callback_;
    MessageCallback message_callback_;

};