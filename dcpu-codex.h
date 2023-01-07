#pragma once
#include <vector>
#include <string>
#include <dcpu-types.h>

using std::string;
using std::vector;

class Codex
{
public:
    static string ValueToStr(Value v, bool isA, word_t nextword);
    static string OpCodeToStr(OpCode op);
    static vector<word_t> PackBytes(const vector<byte_t>& buffer);
    static vector<byte_t> UnpackBytes(const vector<word_t>& buffer);
    static vector<word_t> Encode(const vector<Instruction>& instructions);
    static vector<Instruction> Decode(const vector<word_t>& buffer);
    static Instruction Decode(const word_t* codebyte, word_t maxlen);
};
