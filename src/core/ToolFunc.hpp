#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <sstream>
#include <iomanip>

#if defined(__SSE4_2__) && (defined(__x86_64__) || defined(__i386__))
#include <nmmintrin.h>
#endif

#if defined(__ARM_FEATURE_CRC32)
#include <arm_acle.h>
#endif

//用于检查流
std::string ToHex(const std::string& data) {
    std::stringstream ss;
    // 设置格式：16进制，大写，填充0
    ss << std::hex << std::uppercase << std::setfill('0');
    
    for (unsigned char c : data) {
        // setw(2) 保证像 0x5 打印成 05
        ss << std::setw(2) << static_cast<int>(c) << " ";
    }
    return ss.str();
}

inline uint32_t Crc32C(std::span<const std::byte> bytes) {
    const auto* data = reinterpret_cast<const uint8_t*>(bytes.data());
    size_t len = bytes.size();

#if defined(__SSE4_2__) && (defined(__x86_64__) || defined(__i386__))
    uint64_t crc = 0xFFFFFFFFu;

#if defined(__x86_64__)
    while (len >= sizeof(uint64_t)) {
        uint64_t v;
        std::memcpy(&v, data, sizeof(v));
        crc = _mm_crc32_u64(crc, v);
        data += sizeof(uint64_t);
        len -= sizeof(uint64_t);
    }
#endif

    while (len >= sizeof(uint32_t)) {
        uint32_t v;
        std::memcpy(&v, data, sizeof(v));
        crc = _mm_crc32_u32(static_cast<uint32_t>(crc), v);
        data += sizeof(uint32_t);
        len -= sizeof(uint32_t);
    }

    while (len--) {
        crc = _mm_crc32_u8(static_cast<uint32_t>(crc), *data++);
    }

    return static_cast<uint32_t>(crc ^ 0xFFFFFFFFu);
#elif defined(__ARM_FEATURE_CRC32)
    uint32_t crc = 0xFFFFFFFFu;

    while (len >= sizeof(uint32_t)) {
        uint32_t v;
        std::memcpy(&v, data, sizeof(v));
        crc = __crc32cw(crc, v);
        data += sizeof(uint32_t);
        len -= sizeof(uint32_t);
    }

    while (len--) {
        crc = __crc32cb(crc, *data++);
    }

    return crc ^ 0xFFFFFFFFu;
#else
    uint32_t crc = 0xFFFFFFFFu;
    constexpr uint32_t poly = 0x82F63B78u;

    while (len--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) {
            const uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (poly & mask);
        }
    }

    return crc ^ 0xFFFFFFFFu;
#endif
}