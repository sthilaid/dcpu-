#include <dcpu.h>
#include <dcpu-mem.h>
#include <dcpu-codex.h>
#include <dcpu-hardware-clock.h>
#include <dcpu-hardware-monitor.h>
#include <vector>
#include <fstream>

using std::vector;

uint16_t load_program(Memory& mem, const vector<uint8_t>& codebytes) {
    vector<uint16_t> codeWords = Codex::PackBytes(codebytes);
    vector<Instruction> instructions = Codex::Decode(codeWords);
    return mem.LoadProgram(codeWords);
}

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu <program-bin-file>\n");
        return 1;
    }

    vector<uint8_t> rawbytes;
    std::ifstream binFileStream(args[1], std::ios::binary);
    if (!binFileStream.is_open()) {
        printf("failed to open file: %s\n", args[1]);
        return 1;
    } 
    while(!binFileStream.eof()){
        uint8_t& word = rawbytes.emplace_back();
        binFileStream.read(reinterpret_cast<char*>(&word), 1);
    }
    binFileStream.close();

    Memory mem;
    DCPU cpu;
    cpu.addDevice<Clock>();
    cpu.addDevice<Monitor>();
    const uint16_t lastProgramAddr = load_program(mem, rawbytes);

    while(cpu.getPC() < lastProgramAddr) {
        cpu.step(mem);
    }
    cpu.printRegisters();
    mem.Dump(0xFFF0, 0xFFFF);

    return 0;
}
