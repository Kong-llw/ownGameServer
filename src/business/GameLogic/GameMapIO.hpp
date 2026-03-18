//地图的 磁盘读写
#pragma once

#include <shared_mutex>
#include <unordered_map>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include "RuntimeData.hpp"
constexpr const char* MAP_DATA_DIR = "/data/maps/";
namespace fs = std::filesystem;
namespace GameMap{

    struct MapCache{ //后面应该优化存储量
        std::shared_mutex mtx;
        std::unordered_map<std::string, MapData> maps; // map_name -> MapData
    };
    MapCache& cache();

    const MapData* LoadMapData(const std::string& map_path);
    fs::path GetMapFilePath(const std::string& map_name);
    nlohmann::json LoadMapFile(const fs::path& file_path);
    MapData ParseMapData(const nlohmann::json& root);
    TileType ParseTileType(const nlohmann::json& node);
    void testPrintMap(const MapData& data);
}