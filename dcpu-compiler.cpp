#include <cstdio>
#include <dcpu-codex.h>
#include <dcpu-lispasm.h>
#include <vector>
#include <fstream>

vector<Instruction> GenerateTestInstructions() {
    vector<Instruction> instructions;
    instructions.push_back(Instruction{OpCode_SET, Value_Register_X, Value_NextLitteral, 0, 15});
    instructions.push_back(Instruction{OpCode_ADD, Value_Register_Y, Value_Register_X});
    instructions.push_back(Instruction{OpCode_MUL, Value_Register_X, Value_Register_X});
    return instructions;
}

void testEncodeDecode() {
    vector<Instruction> instructions = GenerateTestInstructions();

    for (const Instruction& i : instructions){
        printf("%s\n", i.toStr().c_str());
    }
    
    vector<uint16_t> binary = Codex::Encode(instructions);
    for (uint16_t b : binary) {
        printf("0x%04X, ", b);
    }
    printf("\n");

    vector<uint8_t> unpackedBinary = Codex::UnpackBytes(binary);
    for (uint8_t b : unpackedBinary) {
        printf("0x%02X, ", b);
    }
    printf("\n");

    vector<Instruction> instructionsFromBinary = Codex::Decode(binary);
    for (const Instruction& i : instructionsFromBinary){
        printf("%s\n", i.toStr().c_str());
    }
}

int main(int argc, char** args) {
    if (argc != 3) {
        printf("usage: dcpu-compiler <input-source-file> <binary-output-file>\n");
        return 1;
    }

    vector<Instruction> instructions = LispAsmParser::ParseLispAsm(args[1]);

    vector<uint16_t> rawcode;
    std::ofstream binFileStream(args[2], std::ios::binary);

    printf("comiling instructions:\n");
    for(const Instruction& i : instructions) printf("  %s\n", i.toStr().c_str());
    vector<uint16_t> codebytes = Codex::Encode(instructions);
    vector<uint8_t> rawdata = Codex::UnpackBytes(codebytes);
    for (uint8_t d : rawdata) {
        binFileStream.write(reinterpret_cast<char*>(&d), 1);
    }
    
    binFileStream.close();
    printf("wrote %d bytes into file %s successfully.\n", rawdata.size(), args[2]);

    return 0;
}
