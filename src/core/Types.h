#pragma once

#include <cstdint>

// 用户ID
using UserId     = std::uint32_t;
// 会话ID（网络连接唯一ID）
using SessionId  = std::uint32_t;
// 组ID（通用组概念，房间是其一种）
using GroupId    = std::uint32_t;
using RoomId     = GroupId;

using MsgId      = std::uint32_t;