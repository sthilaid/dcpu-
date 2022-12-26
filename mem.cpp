#include <mem.h>

Memory::Memory() {
    std::memset(this, 0, TotalBytes);
}

uint16_t Memory::LoadProgram(const vector<uint16_t>& codebytes){
    for(uint16_t addr=0; addr < codebytes.size(); ++addr) {
        // printf("codemem[%04X] = %04X\n", addr, codebytes[addr]);
        m_Buffer[addr] = codebytes[addr];
    }
    return codebytes.size();
}

void Memory::Dump(uint16_t from, uint16_t to) const {
    if (from < 0 || to > LastValidAddress || from > to)
        return;

    uint16_t range = to - from;
    for (uint16_t i=0; i<=range; ++i) {
        uint16_t addr = from+i;
        printf("0x%04X: %04X\n", addr, m_Buffer[addr]);
    }
}

void Memory::DumpNonNull() const {
    for (uint16_t addr=0; addr <= LastValidAddress; ++addr) {
        if (m_Buffer[addr] != 0) {
            printf("0x%04X: %04X\n", addr, m_Buffer[addr]);
        }
    }
}
