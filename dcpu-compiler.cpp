#include <cstdio>
#include <decoder.h>
#include <vector>
#include <fstream>

vector<Instruction> GenerateTestInstructions() {
    vector<Instruction> instructions;
    instructions.push_back(Instruction{OpCode_SET, Value_Register_X, Value_Next, 0, 15});
    instructions.push_back(Instruction{OpCode_ADD, Value_Register_Y, Value_Register_X});
    return instructions;
}

void testEncodeDecode() {
    vector<Instruction> instructions = GenerateTestInstructions();

    for (const Instruction& i : instructions){
        printf("%s\n", i.toStr().c_str());
    }
    
    vector<uint8_t> binary = Decoder::Encode(instructions);
    for (uint8_t b : binary) {
        printf("0x%04X, ", b);
    }
    printf("\n");

    vector<Instruction> instructionsFromBinary = Decoder::Decode(binary);
    for (const Instruction& i : instructionsFromBinary){
        printf("%s\n", i.toStr().c_str());
    }
}

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu-compiler <output-file>\n");
        return 1;
    }

    vector<uint16_t> rawcode;
    std::ofstream binFileStream(args[1], std::ios::binary);

    vector<Instruction> instructions = GenerateTestInstructions();
    printf("comiling instructions:\n");
    for(const Instruction& i : instructions) printf("  %s\n", i.toStr().c_str());
    vector<uint8_t> rawdata = Decoder::Encode(instructions);
    for (uint8_t d : rawdata) {
        binFileStream.write(reinterpret_cast<char*>(&d), 1);
    }
    
    binFileStream.close();
    printf("wrote %d bytes into file %s successfully.\n", rawdata.size(), args[1]);

    return 0;
}
