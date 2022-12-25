#include <dcpu.h>
#include <mem.h>
#include <decoder.h>
#include <vector>
#include <fstream>

using std::vector;

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu <program-bin-file>\n");
        return 1;
    }

    vector<uint8_t> rawcode;
    std::ifstream binFileStream(args[1], std::ios::binary);
    if (!binFileStream.is_open()) {
        printf("failed to open file: %s\n", args[1]);
        return 1;
    } else {
        
        while(!binFileStream.eof()){
            uint8_t& word = rawcode.emplace_back();
            binFileStream.read(reinterpret_cast<char*>(&word), 1);
        }
    }
    binFileStream.close();

    // for(int i=0; i<rawcode.size(); ++i) printf("%02X, ", rawcode[i]); printf("\n");
    vector<uint16_t> packedBytes = Decoder::PackBytes(rawcode);
    // for(int i=0; i<packedBytes.size(); ++i) printf("%04X, ", packedBytes[i]); printf("\n");
    vector<Instruction> instructions = Decoder::Decode(packedBytes);
    printf("decoded instructions:\n");
    for (const Instruction& i : instructions)
        printf("  %s\n", i.toStr().c_str());
    
    Memory mem;
    DCPU cpu;
    mem.LoadProgram(packedBytes);

    for (int i=0; i<instructions.size(); ++i) {
        cpu.Step(mem);
        cpu.PrintRegisters();
    }
    return 0;
}
