#include "TcpConnection.hpp"
#include <asio.hpp>

#include <cstring>
#include <utility>


void TcpConnection::Start() {
    auto self = shared_from_this();
    asio::dispatch(strand_, [this, self]() {
        if (started_ || closed_) {
            return;
        }
        started_ = true;
        DoRead();
    });
}

void TcpConnection::Send(const std::vector<std::byte>& data) {
    Send(std::vector<std::byte>(data));
}

void TcpConnection::Send(std::vector<std::byte>&& data) {
    auto self = shared_from_this();
    asio::dispatch(strand_, [this, self, payload = std::move(data)]() mutable {
        if (closed_) {
            return;
        }

        const bool write_in_progress = !writeQ_.empty();
        writeQ_.push(std::move(payload));
        if (!write_in_progress) {
            DoWrite();
        }
    });
}

void TcpConnection::Close() {
    auto self = shared_from_this();
    asio::dispatch(strand_, [this, self]() {
        DoClose(true);
    });
}

void TcpConnection::DoRead() {
    if (closed_) {
        return;
    }

    auto self = shared_from_this();
    socket_.async_read_some(asio::buffer(read_buffer_),
        asio::bind_executor(strand_, [this, self](std::error_code ec, std::size_t length) {
            if (ec) {
                DoClose(true);
                return;
            }

            if (message_callback_) {
                //std::vector<std::byte> payload(length);
                //std::memcpy(payload.data(), read_buffer_.data(), length);
                message_callback_(std::span<std::byte>(read_buffer_.data(), length));
            }

            DoRead();
        }));
}

void TcpConnection::DoClose(bool notify_close_cb) {
    if (closed_) {
        return;
    }
    closed_ = true;

    std::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);

    std::queue<std::vector<std::byte>> empty;
    writeQ_.swap(empty);

    if (notify_close_cb && close_callback_) {
        auto cb = std::move(close_callback_);
        close_callback_ = nullptr;
        cb();
    }
}

void TcpConnection::DoWrite() {
    if (closed_ || writeQ_.empty()) {
        return;
    }

    auto self = shared_from_this();
    const auto& data = writeQ_.front();
    asio::async_write(socket_, asio::buffer(data), asio::bind_executor(strand_,
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (ec) {
                DoClose(true);
                return;
            }
            writeQ_.pop();
            if (!writeQ_.empty()) {
                DoWrite();
            }
        }));
}
