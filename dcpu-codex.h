#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <dcpu-types.h>

using std::string;
using std::vector;

class Codex
{
public:
    static string ValueToStr(Value v, bool isA, uint16_t nextword);
    static string OpCodeToStr(OpCode op);
    static vector<uint16_t> PackBytes(const vector<uint8_t>& buffer);
    static vector<uint8_t> UnpackBytes(const vector<uint16_t>& buffer);
    static vector<uint16_t> Encode(const vector<Instruction>& instructions);
    static vector<Instruction> Decode(const vector<uint16_t>& buffer);
    static Instruction Decode(const uint16_t* codebyte, uint32_t maxlen);
};
