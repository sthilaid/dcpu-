#pragma once

#include <dcpu-types.h>
#include <cstdio>
#include <cstring>
#include <vector>

using std::vector;

class Memory {
public:
    static constexpr word_t WordByteCount = 2;
    static constexpr word_t LastValidAddress = 0xFFFF;
    static constexpr long_t TotalBytes = (LastValidAddress+1)*WordByteCount;

    Memory();
    word_t LoadProgram(const vector<word_t>& codebytes);
    void Dump(word_t from=0, word_t to=LastValidAddress) const;
    void DumpNonNull() const;

    word_t* operator+(word_t addr) { return m_Buffer+addr; }
    word_t operator[](word_t addr) const { return m_Buffer[addr]; }
    word_t& operator[](word_t addr) { return m_Buffer[addr]; }
    
private:
    word_t m_Buffer[LastValidAddress+1];
};
