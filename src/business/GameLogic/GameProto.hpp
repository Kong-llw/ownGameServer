#pragma once
//游戏过程强相关的一些结构、枚举

#include <vector>
#include <string>
#include "core/Types.h"
#include "business/GamePlayer/GamePlayerInfo.hpp"

struct MatchInfo{
    std::vector<GamePlayerInfo> players;
    std::string selected_map_path;
    UserId owner_id;
};

enum class FSMState : uint8_t {
    Init,       //导入数据
    MapLoad,    //配置地图
    WaitForPlayers, //就绪,等待其他玩家
    BattleIntroAnimation, // 入场动画
    
    //战斗循环
    TurnStart, //重置数据 显示效果等 触发结算之类的
    TurnWaitInput, //接收输入
    TurnActionResolving, //
    EXPAND,
    GROW,
    TurnEndProcessing,
    BattleCheckResult,
    END,
    BattleCleanup,
};

enum class BattleCmdType {
    Move = 0,
    Attack = 1,
    Grow = 2,
};

struct BattleCmd
{ 
    uint64_t player_id; 
    BattleCmdType type;
    int32_t param1; // 例如：Move 的目标位置 x 坐标，Attack 的目标玩家 ID，Grow 的生长量
    int32_t param2; // 例如：Move 的目标位置 y 坐标 
};

struct CmdResult {
    bool success;
    std::string message; //json格式返回内容
};