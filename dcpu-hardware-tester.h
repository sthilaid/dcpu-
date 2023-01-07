#pragma once
#include <dcpu-hardware.h>

//
// device meant only to test the API
//
class TesterDevice : public Hardware {
public:
    TesterDevice();
    cycles_t update(DCPU& cpu, Memory& mem) override;
    cycles_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    word_t m_lastkey = 0;
};
