#pragma once
#include <cstdint>

class DCPU;
class Memory;

class Hardware {
public:
    void init(DCPU& cpu, Memory& mem);
    virtual uint8_t update() = 0;
    virtual uint8_t interrupt() = 0;
    uint32_t getId() const { return m_id; }
    uint16_t getVersion() const { return m_version; }
    uint32_t getManifacturer() const { return m_manifacturer; }
private:
    DCPU* m_cpu = nullptr;
    Memory* m_mem = nullptr;
    uint32_t m_id = 0;
    uint16_t m_version = 0;
    uint32_t m_manifacturer = 0;
};
