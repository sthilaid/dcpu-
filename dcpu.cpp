#include <dcpu.h>

DCPU::DCPU()
    : m_pc {0}
    , m_sp {Memory::LastValidAddress}
    , m_ex {0}
    , m_ia {0}
{
    memset(&m_registers, 0, RegistersCount*2);
}
