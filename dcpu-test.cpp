#include <cassert>
#include <dcpu.h>
#include <dcpu-lispasm.h>
#include <decoder.h>
#include <sstream>

#define CreateTestCase(source, ...)             \
    {                                           \
        TestCase t(source);                     \
        __VA_ARGS__                             \
        t.TryTest();                            \
    }

#define Verify(verif) t.AddVerifier([](const DCPU& cpu, const Memory& mem) { return verif; }, \
                                    [](const DCPU& cpu, const Memory& mem){ return string{#verif}; });
#define VerifyEqual(a, b) t.AddVerifier([](const DCPU& cpu, const Memory& mem) { return a == b; }, \
                                        [](const DCPU& cpu, const Memory& mem) { \
                                            const char fmt[] = #a" (0x%04X) == "#b" (0x%04X)"; \
                                            int sz = snprintf(NULL, 0, fmt, a, b); \
                                            char buf[sz + 1];           \
                                            snprintf(buf, sizeof buf, fmt, a, b); \
                                            return string(buf); });

class TestCase {
public:
    using VerifyType = bool(*)(const DCPU& cpu, const Memory& mem);
    using VerifyStrFnType = string(*)(const DCPU& cpu, const Memory& mem);

    string m_lasmSource = "";
    vector<VerifyType> m_verifiers;
    vector<VerifyStrFnType> m_verifiersTxt;
    int m_testid = 0;
    static int s_id;

    TestCase(string source) : m_lasmSource(source), m_verifiers(), m_verifiersTxt(), m_testid(s_id++) {}
    void AddVerifier(VerifyType v, VerifyStrFnType vStr) {
        m_verifiers.push_back(v);
        m_verifiersTxt.push_back(vStr);
    }
    bool TryTest() const;
};

int TestCase::s_id = 0;

bool TestCase::TryTest() const {
    std::basic_stringstream sourceStream{m_lasmSource};
    vector<Token> tokens = LispAsmParser::Tokenize(sourceStream);
    vector<Instruction> instructions = LispAsmParser::ParseTokens(tokens);
    vector<uint8_t> codebytes = Decoder::UnpackBytes(Decoder::Encode(instructions));

    bool test_success = true;
    DCPU cpu;
    Memory mem;
    cpu.Run(mem, codebytes);
    for (int i=0; i < m_verifiers.size(); ++i) {
        bool success = m_verifiers[i](cpu, mem);
        if (success)
            printf("Test %d-%d [SUCCESS]\n", m_testid, i);
        else {
        failedtest:
            printf("Test %d-%d [FAILURE] : %s\n", m_testid, i, m_verifiersTxt[i](cpu, mem).c_str());
        }
        test_success &= success;
    }

    return test_success;
}

int main(int argc, char** argv) {
    CreateTestCase("(set X 12)\n", Verify(cpu.GetRegister(Registers_X) == 12));

    CreateTestCase("(set X 12)\n"
                   "(set (ref x) 21)", Verify(mem[12] == 21));

    CreateTestCase("(set push 14)\n"
                   "(add peek 1)"
                   "(set b 0x7)"
                   "(and b pop)"
                   "(set a (ref sp -1))",
                   Verify(cpu.GetSP() == 0xFFFF)
                   Verify(cpu.GetRegister(Registers_B) == 7)
                   Verify(mem[0xFFFE] == 15)
                   Verify(cpu.GetRegister(Registers_A) == 15)
                   );

    CreateTestCase("(set x 0xFFFF)"
                   "(add x 1)",
                   VerifyEqual(cpu.GetEX(), 1)
                   VerifyEqual(cpu.GetRegister(Registers_A), 0)
                   );

    CreateTestCase("(set (ref 555) 10)"
                   "(sub (ref 555) 1)"
                   "(set y 10)"
                   "(sub y 1)"
                   "(sub x 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 0xFFFF)
                   VerifyEqual(cpu.GetEX(), 0xFFFF)
                   VerifyEqual(cpu.GetRegister(Registers_X), static_cast<uint16_t>(-1))
                   VerifyEqual(cpu.GetRegister(Registers_Y), 9)
                   VerifyEqual(mem[555], 9)
                   );

    CreateTestCase("(set x 3)"
                   "(mul x x)"
                   "(mul x x)"
                   "(set y 0x8000)"
                   "(mul y 2)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 81)
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetEX(), 1)
                   );
}
