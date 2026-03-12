# ========== 收集源码（初期空文件，后续补充） ==========
# 基础层
file(GLOB_RECURSE CORE_SRCS CONFIGURE_DEPENDS "core/*.h" "core/*.hpp" "core/*.cpp")
# 网络层
file(GLOB_RECURSE NETWORK_SRCS CONFIGURE_DEPENDS "network/*.h" "network/*.hpp" "network/*.cpp")
# 协议层
file(GLOB_RECURSE PROTOCOL_SRCS CONFIGURE_DEPENDS "protocol/*.h" "protocol/*.hpp" "protocol/*.cpp")
# 业务层
file(GLOB_RECURSE BUSINESS_SRCS CONFIGURE_DEPENDS "business/*.h" "business/*.hpp" "business/*.cpp")

# ========== 构建可执行程序 ==========
add_executable(game_server
    main.cpp          # 程序入口（后续写）
    ${CORE_SRCS}
    ${NETWORK_SRCS}
    ${PROTOCOL_SRCS}
    ${BUSINESS_SRCS}
)

# ========== 链接依赖库 ==========
target_link_libraries(game_server
    PRIVATE
        spdlog::spdlog          # 日志库
        nlohmann_json::nlohmann_json # JSON库
        pthread                 # Linux线程库（Asio需要）
)

# ========== 头文件路径 ==========
target_include_directories(game_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}          # 源码根目录
        ${asio_SOURCE_DIR}/asio/include      # Asio头文件
        ${CMAKE_SOURCE_DIR}/config           # 配置文件目录
)

# ========== 编译器选项（警告+优化） ==========
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(game_server PRIVATE -Wall -Wextra -Werror -O2)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)