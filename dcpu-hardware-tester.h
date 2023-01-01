#pragma once
#include <dcpu-hardware.h>

//
// device meant only to test the API
//
class TesterDevice : public Hardware {
public:
    TesterDevice();
    uint32_t update(DCPU& cpu, Memory& mem) override;
    uint32_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    uint16_t m_lastkey = 0;
};
