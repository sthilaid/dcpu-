#include <dcpu.h>
#include <dcpu-mem.h>
#include <dcpu-codex.h>
#include <dcpu-hardware-clock.h>
#include <dcpu-hardware-monitor.h>
#include <vector>
#include <fstream>

using std::vector;

word_t load_program(Memory& mem, const vector<byte_t>& codebytes) {
    vector<word_t> codeWords = Codex::PackBytes(codebytes);
    vector<Instruction> instructions = Codex::Decode(codeWords);
    return mem.LoadProgram(codeWords);
}

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu <program-bin-file>\n");
        return 1;
    }

    vector<byte_t> rawbytes;
    std::ifstream binFileStream(args[1], std::ios::binary);
    if (!binFileStream.is_open()) {
        printf("failed to open file: %s\n", args[1]);
        return 1;
    } 
    while(!binFileStream.eof()){
        byte_t& halfword = rawbytes.emplace_back();
        binFileStream.read(reinterpret_cast<char*>(&halfword), 1);
    }
    binFileStream.close();

    Memory mem;
    DCPU cpu;
    cpu.addDevice<Clock>();
    cpu.addDevice<Monitor>();
    const word_t lastProgramAddr = load_program(mem, rawbytes);

    while(cpu.getPC() < lastProgramAddr) {
        cpu.step(mem);
    }
    cpu.printRegisters();
    mem.Dump(0xFFF0, 0xFFFF);

    return 0;
}
