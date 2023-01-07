#include <dcpu-mem.h>

Memory::Memory() {
    std::memset(this, 0, TotalBytes);
}

word_t Memory::LoadProgram(const vector<word_t>& codebytes){
    for(word_t addr=0; addr < codebytes.size(); ++addr) {
        // printf("codemem[%04X] = %04X\n", addr, codebytes[addr]);
        m_Buffer[addr] = codebytes[addr];
    }
    return codebytes.size();
}

void Memory::Dump(word_t from, word_t to) const {
    if (from < 0 || to > LastValidAddress || from > to)
        return;

    word_t range = to - from;
    for (word_t i=0; i<=range; ++i) {
        word_t addr = from+i;
        printf("0x%04X: %04X\n", addr, m_Buffer[addr]);
    }
}

void Memory::DumpNonNull() const {
    for (word_t addr=0; addr <= LastValidAddress; ++addr) {
        if (m_Buffer[addr] != 0) {
            printf("0x%04X: %04X\n", addr, m_Buffer[addr]);
        }
    }
}
