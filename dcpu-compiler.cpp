#include <cstdio>
#include <dcpu-codex.h>
#include <dcpu-lispasm.h>
#include <vector>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

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
    string infile = argc >= 2 ? args[1] : "";
    string outfile = infile;
    if (argc >= 3) {
        outfile = args[2];
    } else if (argc == 2 && infile.compare(infile.size()-5, infile.size(), ".lasm") == 0) {
        outfile.replace(infile.size()-5, infile.size(), ".dcpu");;
    }
    if (outfile.empty()) {
        printf("usage: dcpu-compiler <input-source-file.lasm>\n");
        printf("usage: dcpu-compiler <input-source-file> binary-output-file\n");
        return 1;
    }

    if (!fs::exists(fs::status(infile.c_str()))) {
        printf("Unknown file: %s\n", infile.c_str());
        return 1;
    }

    vector<Instruction> instructions = LispAsmParser::ParseLispAsm(infile.c_str());

    vector<uint16_t> rawcode;
    std::ofstream binFileStream(outfile.c_str(), std::ios::binary);

    printf("comiling instructions:\n");
    for(const Instruction& i : instructions) printf("  %s\n", i.toStr().c_str());
    vector<uint16_t> codebytes = Codex::Encode(instructions);
    vector<uint8_t> rawdata = Codex::UnpackBytes(codebytes);
    for (uint8_t d : rawdata) {
        binFileStream.write(reinterpret_cast<char*>(&d), 1);
    }
    
    binFileStream.close();
    printf("wrote %d bytes into file %s successfully.\n", rawdata.size(), outfile.c_str());

    return 0;
}
