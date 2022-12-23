#pragma once

#include <cstdint>
#include <mem.h>

enum class Registers : uint16_t {
    A,
    B,
    C,
    X,
    Y,
    Z,
    I,
    J,
    Count,
};
static constexpr uint16_t RegistersCount = static_cast<uint16_t>(Registers::Count);

class DCPU {

public:
    DCPU();
    uint32_t Step(Memory& mem);

private:
    uint16_t m_pc;
    uint16_t m_sp;
    uint16_t m_ex;
    uint16_t m_ia;
    uint16_t m_registers[RegistersCount];
};

