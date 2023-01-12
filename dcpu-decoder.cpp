#include <dcpu-types.h>
#include <dcpu-codex.h>
#include <vector>
#include <fstream>

using std::vector;

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu-decoder <program-bin-file>\n");
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

    printf("Decoding instructions:\n");

    const vector<word_t> packedBytes = Codex::PackBytes(rawbytes);
    const vector<Instruction> instructions = Codex::Decode(packedBytes);
    uint8_t addr=0;
    for (const Instruction& inst : instructions) {
        printf("0x%04X - %s\n", addr, inst.toStr().c_str());
        addr += inst.WordCount();
    }
    printf("validating %d code words out of %d expected\n", addr, packedBytes.size());
    
    return 0;
}
