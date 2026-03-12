#pragma once
#include <vector>
#include <string>
#include "core/Types.h"
#include "business/GamePlayer/GamePlayerInfo.hpp"

struct MatchInfo{
    std::vector<GamePlayerInfo> players;
    std::string selected_map_path;
    UserId owner_id;
};