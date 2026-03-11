# game_server/test/CMakeLists.txt
# ========== 收集测试源码 ==========
file(GLOB UNIT_TEST_SRCS "unit/*.h" "unit/*.cpp")

# ========== 构建测试可执行程序 ==========
add_executable(unit_tests ${UNIT_TEST_SRCS})

# ========== 链接依赖 ==========
target_link_libraries(unit_tests
    PRIVATE
        gtest_main          # GTest主函数（无需自己写）
        gmock               # 模拟工具（可选）
        spdlog::spdlog      # 日志库
        nlohmann_json::nlohmann_json
)

# ========== 头文件路径 ==========
target_include_directories(unit_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src              # 业务源码目录
        ${asio_SOURCE_DIR}/asio/include      # Asio头文件
)

# ========== 注册测试 ==========
include(GoogleTest)
gtest_discover_tests(unit_tests)

set_target_properties(unit_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)