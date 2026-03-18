#include "GameMapIO.hpp"
#include <mutex>
#include <memory>
#include <fstream>
#include <iostream>
using nlohmann::json;

namespace GameMap{
    MapCache& cache() {
        static MapCache instance;
        return instance;
    }

    const MapData* LoadMapData(const std::string& map_path){
        auto& c = cache();
        {
            std::shared_lock lock(c.mtx);
            auto it = c.maps.find(map_path);
            if(it != c.maps.end()){
                return &it->second;
            }
        }

        try{
            fs::path file_path = GetMapFilePath(map_path);
            json root = LoadMapFile(file_path);
            MapData new_data = ParseMapData(root);
            std::unique_lock lock(c.mtx);
            auto[insert_it, inserted] = c.maps.try_emplace(map_path, std::move(new_data));
            return &insert_it->second;
        }
        catch(const std::exception& e){
            //LOG("Failed to load map data: " + std::string(e.what()));
            return nullptr;
        }
    }

    fs::path GetMapFilePath(const std::string& map_name){
        return fs::path(MAP_DATA_DIR) / (map_name + ".json");
    }

    nlohmann::json LoadMapFile(const fs::path& file_path){
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open map file: " + file_path.string());
        }
        json root;
        file >> root;
        return root;
    }

    MapData ParseMapData(const nlohmann::json& root){
        MapData data;
        data.width = root.at("width").get<uint32_t>();
        data.height = root.at("height").get<uint32_t>();
        const auto tiles_node = root.at("tiles");
        data.tiles.reserve(tiles_node.size());
        for (const auto& tile_node : tiles_node) {
            TileData tile;
            tile.tile_id = tile_node.at("tile_id").get<uint32_t>();
            tile.owner = tile_node.value("owner", 0u);
            tile.value = tile_node.value("value", 0u);
            tile.type = ParseTileType(tile_node.at("type"));
            tile.round_type = static_cast<uint8_t>(tile_node.value("round_type", 0u));
            tile.countered = tile_node.value("countered", false);
            tile.acted = tile_node.value("acted", false);
            tile.isDisconnected = tile_node.value("isDisconnected", false);
            data.tiles.push_back(std::move(tile));
        }
        return data;
    }
    TileType ParseTileType(const nlohmann::json& node){
        std::string type_str = node.get<std::string>();
        if(type_str == "Mountain") return TileType::Mountain;
        if(type_str == "Water") return TileType::Water;
        if(type_str == "Forest") return TileType::Forest;
        if(type_str == "Ground") return TileType::Ground;
        if(type_str == "QMark") return TileType::QMark;
        if(type_str == "EMark") return TileType::EMark;
        if(type_str == "BlackBg") return TileType::BlackBg;
        if(type_str == "Base") return TileType::Base;
        return TileType::BlackBg;
    }

    void testPrintMap(const MapData& data){
        for(uint32_t y = 0; y < data.height; ++y){
            for(uint32_t x = 0; x < data.width; ++x){
                const TileData& tile = data.tiles[y * data.width + x];
                char c = 'X';
                switch(tile.type){
                    case TileType::Mountain: c = 'M'; break;
                    case TileType::Water: c = 'W'; break;
                    case TileType::Forest: c = 'F'; break;
                    case TileType::Ground: c = 'G'; break;
                    case TileType::QMark: c = '?'; break;
                    case TileType::EMark: c = '!'; break;
                    case TileType::BlackBg: c = ' '; break;
                    case TileType::Base: c = 'B'; break;
                }
                std::cout << c << ' ';
            }
            std::cout << '\n';
        }
    }
}