#pragma once

#include <atomic>
#include <asio.hpp>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/Types.h"
#include "ClientSession.hpp"
#include "protocol/Router/IBusinessMsgGateway.hpp"

namespace Network {
class UserSessionMap;
}

namespace Network {

class SessionManager : public std::enable_shared_from_this<SessionManager> {
public:
    static constexpr std::size_t kMaxConnectionNum = 1000;

    explicit SessionManager(asio::any_io_executor exec,
         std::shared_ptr<UserSessionMap> user_session_map,
         std::shared_ptr<IBusinessMsgGateway> gateway);
    ~SessionManager() = default;

    using SessionCloseHandler = std::function<void(std::shared_ptr<ClientSession>)>;
    void SetOnSessionClose(SessionCloseHandler cb);

    SessionId AllocateSessionId();

    SessionId CreateSession(tcp::socket socket, std::shared_ptr<IMessageCodec> codec);
    bool CloseSession(SessionId session_id);
    bool IsConnectionFull() const;
    std::shared_ptr<ClientSession> GetSession(SessionId session_id) const;

    template <typename Callable> requires std::is_invocable_r_v<void, Callable, std::shared_ptr<ClientSession>>
    void forEachSession(Callable&& func) {
        std::vector<std::shared_ptr<ClientSession>> snapshot;
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            snapshot.reserve(sessions_.size());
            for (const auto& [sid, session] : sessions_) {
                (void)sid;
                snapshot.push_back(session);
            }
        }
        for (const auto& session : snapshot) {
            func(session);
        }
    }

private:
    std::unordered_map<SessionId, std::shared_ptr<ClientSession>> sessions_;
    mutable std::shared_mutex mutex_;
    std::atomic<SessionId> next_session_id_{1};
    asio::any_io_executor executor_;
    //std::shared_ptr<UserSessionMap> user_session_map_;
    std::shared_ptr<IBusinessMsgGateway> gateway_;
    SessionCloseHandler on_session_close_;

    SessionManager() = delete;
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
};
}