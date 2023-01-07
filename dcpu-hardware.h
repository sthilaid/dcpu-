#pragma once
#include <dcpu-types.h>

class DCPU;
class Memory;

class Hardware {
public:
    virtual ~Hardware() {};
    void init(deviceIdx_t deviceIndex);
    virtual cycles_t update(DCPU& cpu, Memory& mem) = 0;
    virtual cycles_t interrupt(DCPU& cpu, Memory& mem) = 0;

    long_t getId() const { return m_id; }
    word_t getVersion() const { return m_version; }
    long_t getManifacturer() const { return m_manifacturer; }
protected:
    long_t m_id = 0;
    word_t m_version = 0;
    long_t m_manifacturer = 0;
    word_t m_deviceId = 0;
};
