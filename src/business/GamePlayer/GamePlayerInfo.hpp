#pragma once

#include "core/User/UserBaseInfo.hpp"

struct GamePlayerInfo{
    UserBaseInfo b_info;

    uint8_t seat_index;
    uint8_t color;
    uint8_t position;
    bool ready;
};