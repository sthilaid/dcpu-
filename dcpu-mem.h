#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

using std::vector;

class Memory {
public:
    static constexpr uint16_t WordByteCount = 2;
    static constexpr uint16_t LastValidAddress = 0xFFFF;
    static constexpr uint32_t TotalBytes = (LastValidAddress+1)*WordByteCount;

    Memory();
    uint16_t LoadProgram(const vector<uint16_t>& codebytes);
    void Dump(uint16_t from=0, uint16_t to=LastValidAddress) const;
    void DumpNonNull() const;

    uint16_t* operator+(uint16_t addr) { return m_Buffer+addr; }
    uint16_t operator[](uint16_t addr) const { return m_Buffer[addr]; }
    uint16_t& operator[](uint16_t addr) { return m_Buffer[addr]; }
    
private:
    uint16_t m_Buffer[LastValidAddress+1];
};
