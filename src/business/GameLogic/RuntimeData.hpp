// RuntimeData.hpp
// 运行时数据容器：地图、轮次、玩家状态等（框架占位，逻辑待实现）
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Types.h"
// 描述单个玩家在对局中的基础状态
struct PlayerRuntimeState {
	UserId player_id;
	uint32_t seat_index;
	uint32_t score; //没有被维护，客户端自己算，服务端不管了。
	uint32_t resources;	
	uint32_t baseCount;
	bool failure;
};
enum TileType : unsigned char
{
	Mountain = 1,
	Water = 2,
	Forest = 3,
	Ground = 4,
	QMark = 5,
	EMark = 6,
	BlackBg = 7,
	Base = 8,
};

struct TileData{
	uint32_t tile_id;
	UserId owner;
	uint32_t value;
	TileType type{TileType::BlackBg};
	uint8_t round_type;
	bool countered{false};
	bool acted{false};
	bool isDisconnected{false};
};

struct MapData{
	uint32_t width{0};
	uint32_t height{0};
	std::vector<TileData> tiles;
};

// 运行时的地图与局内状态，后续可扩展具体字段
struct RuntimeData {
	std::string map_name;
	MapData map_data;
	std::unordered_map<UserId, PlayerRuntimeState> players; // 按玩家 ID 存储局内状态

	uint64_t tick {0}; // 帧/回合计数
	UserId current_player_id {0};
};
