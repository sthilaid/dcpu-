#pragma once
#include <cstdint>

class DCPU;
class Memory;

class Hardware {
public:
    virtual ~Hardware() {};
    void init(uint8_t deviceIndex);
    virtual uint32_t update(DCPU& cpu, Memory& mem) = 0;
    virtual uint32_t interrupt(DCPU& cpu, Memory& mem) = 0;

    uint32_t getId() const { return m_id; }
    uint16_t getVersion() const { return m_version; }
    uint32_t getManifacturer() const { return m_manifacturer; }
protected:
    uint32_t m_id = 0;
    uint16_t m_version = 0;
    uint32_t m_manifacturer = 0;
    uint16_t m_deviceId = 0;
};
