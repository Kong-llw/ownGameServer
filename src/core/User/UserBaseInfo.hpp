#pragma once
#include <string>
#include <cstdint>
#include "src/core/Types.h"

struct UserBaseInfo {
    UserId user_id;
    GroupId current_group_id;
    std::string user_name;
    SessionId session_id;
    bool is_online;

    bool IsValid() const {
        return user_id != 0 && !user_name.empty();
    }   
};