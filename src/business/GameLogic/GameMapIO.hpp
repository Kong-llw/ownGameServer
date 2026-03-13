//地图的 磁盘读写
#pragma once

#include "RuntimeData.hpp"
constexpr const char* MAP_DATA_DIR = "./data/maps/";

class GameMapIO {
public:
    static bool SaveMap(const std::string& map_name, const MapData& data); //暂不支持
    static bool LoadMap(const std::string& map_name, MapData& out_data);


};