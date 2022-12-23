#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

struct Memory {
    static constexpr uint16_t WordByteCount = 2;
    static constexpr uint16_t LastValidAddress = 0xFFFF;
    static constexpr uint32_t TotalBytes = (LastValidAddress+1)*WordByteCount;

    Memory();
    void Dump(uint16_t from=0, uint16_t to=LastValidAddress);
    
    uint16_t Buffer[LastValidAddress+1];
};
