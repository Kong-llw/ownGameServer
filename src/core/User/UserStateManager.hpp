#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <mutex>

#include "UserBaseInfo.hpp"

class UserStateManager{
private:
    std::unordered_map<SessionId, UserBaseInfo> user_states;
    std::shared_mutex state_mutex; 

public:
    std::optional<UserBaseInfo> Get(UserId uid) const
    {
        std::shared_lock lock(state_mutex);
        auto it = user_states.find(uid);
        if (it == user_states.end()) return std::nullopt;
        return it->second;
    }

    // 写：独占锁
    bool Update(const UserBaseInfo& info)
    {
        std::unique_lock lock(state_mutex);
        user_states[info.user_id] = info;
        return true;
    }
};  