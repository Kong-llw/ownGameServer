// test/unit/core_test.cpp
#include <gtest/gtest.h>
#include <asio/asio.hpp>
#include <nlohmann/json.hpp>

// 测试Asio基础功能
TEST(AsioTest, IoContextInit) {
    asio::io_context io;
    EXPECT_NO_THROW(io.run()); // 验证io_context能正常运行
}

// 测试JSON库
TEST(JsonTest, BasicParse) {
    nlohmann::json j = {{"name", "game_server"}, {"version", "1.0.0"}};
    EXPECT_EQ(j["name"], "game_server");
    EXPECT_EQ(j["version"], "1.0.0");
}