#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "business/GameLogic/GameMapIO.hpp"

namespace {

TEST(GameMapIOTest, LoadSpecifiedDirectoryFileAndPrintMap) {
    namespace fs = std::filesystem;

    const fs::path temp_dir = fs::temp_directory_path() / "reproject_gamemapio_tests";
    ASSERT_NO_THROW(fs::create_directories(temp_dir));

    const fs::path map_file = temp_dir / "sample_map.json";
    std::ofstream out(map_file);
    ASSERT_TRUE(out.is_open());
    out << R"({
        "width": 2,
        "height": 2,
        "tiles": [
            {"tile_id": 1, "owner": 0, "value": 0, "type": "Mountain", "round_type": 0, "countered": false, "acted": false, "isDisconnected": false},
            {"tile_id": 2, "owner": 0, "value": 0, "type": "Ground", "round_type": 0, "countered": false, "acted": false, "isDisconnected": false},
            {"tile_id": 3, "owner": 0, "value": 0, "type": "Water", "round_type": 0, "countered": false, "acted": false, "isDisconnected": false},
            {"tile_id": 4, "owner": 0, "value": 0, "type": "Base", "round_type": 0, "countered": false, "acted": false, "isDisconnected": false}
        ]
    })";
    out.close();

    const nlohmann::json root = GameMap::LoadMapFile(map_file);
    const MapData data = GameMap::ParseMapData(root);

    EXPECT_EQ(data.width, 2u);
    EXPECT_EQ(data.height, 2u);
    ASSERT_EQ(data.tiles.size(), 4u);
    EXPECT_EQ(data.tiles[0].type, TileType::Mountain);
    EXPECT_EQ(data.tiles[1].type, TileType::Ground);
    EXPECT_EQ(data.tiles[2].type, TileType::Water);
    EXPECT_EQ(data.tiles[3].type, TileType::Base);

    testing::internal::CaptureStdout();
    GameMap::testPrintMap(data);
    const std::string printed = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(printed.empty());
    EXPECT_NE(printed.find("M"), std::string::npos);
    EXPECT_NE(printed.find("G"), std::string::npos);
    EXPECT_NE(printed.find("W"), std::string::npos);
    EXPECT_NE(printed.find("B"), std::string::npos);

    std::error_code ec;
    fs::remove(map_file, ec);
    fs::remove(temp_dir, ec);
}

} // namespace
