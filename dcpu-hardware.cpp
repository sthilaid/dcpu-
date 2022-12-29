#include <dcpu-hardware.h>
#include <dcpu.h>
#include <mem.h>

void Hardware::init(DCPU& cpu, Memory& mem) {
    m_cpu = &cpu;
    m_mem = &mem;
}
