#pragma once
#include <dcpu-hardware.h>

//
// device meant only to test the API
//
class TesterDevice : public Hardware {
public:
    TesterDevice();
    uint8_t update(DCPU& cpu, Memory& mem) override;
    uint8_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    uint16_t m_lastkey = 0;
};
