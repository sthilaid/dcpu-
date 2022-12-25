#include <mem.h>

Memory::Memory() {
    std::memset(this, 0, TotalBytes);
}

void Memory::LoadProgram(const vector<uint16_t>& codebytes){
    for(uint16_t addr=0; addr < codebytes.size(); ++addr) {
        // printf("codemem[%04X] = %04X\n", addr, codebytes[addr]);
        m_Buffer[addr] = codebytes[addr];
    }
}

void Memory::Dump(uint16_t from, uint16_t to) {
    if (from < 0 || to > LastValidAddress || from > to)
        return;

    uint16_t range = to - from;
    for (uint16_t i=0; i<range; ++i) {
        uint16_t addr = from+i;
        printf("0x%04X: %04X\n", addr, m_Buffer[addr]);
    }
}
