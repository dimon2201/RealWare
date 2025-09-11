#pragma once

#include <cstdint>

namespace realware
{
    namespace core
    {
        using boolean = uint32_t;
        using s8 = int8_t;
        using u8 = uint8_t;
        using s16 = int16_t;
        using u16 = uint16_t;
        using s32 = int32_t;
        using u32 = uint32_t;
        using s64 = int64_t;
        using u64 = uint64_t;
        using f32 = float;
        using usize = size_t;
        using dword = u32;
        using qword = u64;

#if defined(__ILP32__) || defined(__arm__) || defined(_M_ARM) || defined(__i386__) || defined(_M_IX86) || defined(_X86_)
        // 32-bit architecture
        constexpr int REALWARE_CPU_ARCHITECTURE = 32;
        using cpuword = dword;
#elif defined(__amd64__) || defined(_M_AMD64) || defined(_M_X64) || defined(__aarch64__) || defined(__ia64__) || defined(_M_IA64)
        // 64-bit architecture
        constexpr int REALWARE_CPU_ARCHITECTURE = 64;
        using cpuword = qword;
#endif

        constexpr boolean K_TRUE = 1;
        constexpr boolean K_FALSE = 0;
    }
}