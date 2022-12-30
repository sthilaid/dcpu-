#include <cassert>
#include <dcpu.h>
#include <dcpu-mem.h>
#include <dcpu-lispasm.h>
#include <dcpu-codex.h>
#include <sstream>
#include <cstdlib>

#define CreateTestCase(name, source, ...)                               \
    {                                                                   \
        TestCase t(name, source);                                       \
        __VA_ARGS__                                                     \
        bool shouldRun = !shouldStop && (singleTestName == nullptr || std::strcmp(singleTestName, t.m_testName) == 0);  \
        if (shouldRun && !t.TryTest()) shouldStop = true;               \
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

    const char* m_testName = nullptr;
    string m_lasmSource = "";
    vector<VerifyType> m_verifiers;
    vector<VerifyStrFnType> m_verifiersTxt;
    int m_id = 0;
    static int s_id;

    TestCase(const char* name, string source)
        : m_testName(name)
        , m_lasmSource(source)
        , m_verifiers()
        , m_verifiersTxt()
        , m_id(s_id++)
    {}
    
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
    vector<uint8_t> codebytes = Codex::UnpackBytes(Codex::Encode(instructions));

    int test_success = 0;
    DCPU cpu;
    Memory mem;
 BeforeRun:
    cpu.Run(mem, codebytes);
    for (int i=0; i < m_verifiers.size(); ++i) {
        bool success = m_verifiers[i](cpu, mem);
        if (!success) {
        failedtest:
            printf("Test %s-%d [FAILURE] : %s\n", m_testName, i, m_verifiersTxt[i](cpu, mem).c_str());
        } else {
            ++test_success;
        }
    }
    printf("Test %s %d/%d [%s]\n", m_testName, test_success, m_verifiers.size(),
           test_success == m_verifiers.size() ? "SUCCESS" : "FAILURE");

    return test_success == m_verifiers.size();
}

int main(int argc, char** argv) {
    const char* singleTestName = nullptr;
    bool shouldStop = false;
    if (argc > 1) {
        singleTestName = argv[1];
    }
    
    CreateTestCase("Basic", "(set X 12)\n", VerifyEqual(cpu.GetRegister(Registers_X), 12));

    CreateTestCase("SET",
                   "(set X 12)\n"
                   "(set (ref x) 21)", VerifyEqual(mem[12], 21));

    CreateTestCase("Basic Various",
                   "(set push 14)\n"
                   "(add peek 1)"
                   "(set b 0x7)"
                   "(and b pop)"
                   "(set a (ref sp -1))",
                   VerifyEqual(cpu.GetSP(), 0xFFFF)
                   VerifyEqual(cpu.GetRegister(Registers_B), 7)
                   VerifyEqual(mem[0xFFFE], 15)
                   VerifyEqual(cpu.GetRegister(Registers_A), 15)
                   );

    CreateTestCase("ADD",
                   "(set x 0xFFFF)"
                   "(add x 1)",
                   VerifyEqual(cpu.GetEX(), 1)
                   VerifyEqual(cpu.GetRegister(Registers_A), 0)
                   VerifyEqual(cpu.GetCycles(), 3)
                   );

    CreateTestCase("SUB",
                   "(set (ref 555) 10)"
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
                   VerifyEqual(cpu.GetCycles(), 10)
                   );

    CreateTestCase("MUL",
                   "(set x 3)"
                   "(mul x x)"
                   "(mul x x)"
                   "(set y 0x8000)"
                   "(mul y 3)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 81)
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0x8000)
                   VerifyEqual(cpu.GetEX(), 0x1)
                   VerifyEqual(cpu.GetCycles(), 9)
                   );

    CreateTestCase("MLU",
                   "(set x -1)"
                   "(set y -1)"
                   "(mul x y)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 1)
                   VerifyEqual(cpu.GetRegister(Registers_Y), static_cast<uint16_t>(-1))
                   VerifyEqual(cpu.GetCycles(), 4)
                   );

    CreateTestCase("DIV",
                   "(set x 29)"
                   "(set y 3)"
                   "(div x y)"
                   "(div y 0)"
                   "(set i 1)"
                   "(set j 0x400)"
                   "(div i j)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 9)
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetEX(), 64)
                   VerifyEqual(cpu.GetCycles(), 14)
                   );

    CreateTestCase("MOD",
                   "(set x 29)"
                   "(set y 3)"
                   "(mod x y)"
                   "(mod y 0)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 2)
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetCycles(), 8)
                   );

    CreateTestCase("MDI",
                   "(set x -29)"
                   "(set y 3)"
                   "(mdi x y)"
                   "(mdi y 0)"
                   "(set i 29)"
                   "(set j 3)"
                   "(mdi i j)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), static_cast<uint16_t>(-2))
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetRegister(Registers_I), 2)
                   VerifyEqual(cpu.GetCycles(), 14)
                   );

    CreateTestCase("AND",
                   "(set x 0xAA)"
                   "(set y 0xF0)"
                   "(and x y)"
                   "(and y 0)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 0xA0);
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetCycles(), 6)
                   );

    CreateTestCase("BOR",
                   "(set x 0xAA)"
                   "(set y 0x55)"
                   "(bor x y)"
                   "(bor y 0xFF)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 0xFF);
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0xFF)
                   VerifyEqual(cpu.GetCycles(), 7)
                   );

    CreateTestCase("XOR",
                   "(set x 0xAA)"
                   "(set y 0x55)"
                   "(xor x y)"
                   "(xor y 0xFF)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 0xFF);
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0xAA)
                   VerifyEqual(cpu.GetCycles(), 7)
                   );

    CreateTestCase("SHR",
                   "(set x 0xAA)"
                   "(set y 0x55)"
                   "(shr x 1)"
                   "(shr y 1)"
                   "(set i 1)"
                   "(shr i 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 0x55);
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0x2A)
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetEX(), 0x8000)
                   VerifyEqual(cpu.GetCycles(), 8)
                   );

    CreateTestCase("ASR",
                   "(set x 1)"
                   "(set y 0x8000)"
                   "(asr x 1)"
                   "(asr y 1)"
                   "(set i 0xF)"
                   "(asr i 3)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 0);
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0xC000)
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetEX(), 0xE000)
                   VerifyEqual(cpu.GetCycles(), 7)
                   );

    CreateTestCase("SHL",
                   "(set x 1)"
                   "(set y 0x8000)"
                   "(shl x 3)"
                   "(shl y 1)"
                   "(set i 0xF)"
                   "(shl i 3)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 8);
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetRegister(Registers_I), 0x78)
                   VerifyEqual(cpu.GetEX(), 0xE000)
                   VerifyEqual(cpu.GetCycles(), 7)
                   );

    CreateTestCase("IFB",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifb x y)"
                   "(set i 1)"
                   "(set x 1)"
                   "(set y 3)"
                   "(ifb x y)"
                   "(set j 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetRegister(Registers_J), 1)
                   VerifyEqual(cpu.GetCycles(), 10)
                   );

    CreateTestCase("IFC",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifc x y)"
                   "(set i 1)"
                   "(set x 1)"
                   "(set y 3)"
                   "(ifc x y)"
                   "(set j 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetRegister(Registers_J), 0)
                   VerifyEqual(cpu.GetCycles(), 10)
                   );

    CreateTestCase("IFE",
                   "(set x 1)"
                   "(set y 2)"
                   "(ife x y)"
                   "(set i 1)"
                   "(set x 3)"
                   "(set y 3)"
                   "(ife x y)"
                   "(set j 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetRegister(Registers_J), 1)
                   VerifyEqual(cpu.GetCycles(), 10)
                   );

    CreateTestCase("IFN",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifn x y)"
                   "(set i 1)"
                   "(set x 3)"
                   "(set y 3)"
                   "(ifn x y)"
                   "(set j 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetRegister(Registers_J), 0)
                   VerifyEqual(cpu.GetCycles(), 10)
                   );

    CreateTestCase("IFG",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifg x y)"
                   "(set i 1)"
                   "(set x 2)"
                   "(set y 2)"
                   "(ifg x y)"
                   "(set j 1)"
                   "(set x 2)"
                   "(set y 1)"
                   "(ifg x y)"
                   "(set a 1)"
                   "(set x 1)"
                   "(set y -1)"
                   "(ifg x y)"
                   "(set b 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetRegister(Registers_J), 0)
                   VerifyEqual(cpu.GetRegister(Registers_A), 1)
                   VerifyEqual(cpu.GetRegister(Registers_B), 0)
                   VerifyEqual(cpu.GetCycles(), 20)
                   );

    CreateTestCase("IFA",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifa x y)"
                   "(set i 1)"
                   "(set x 2)"
                   "(set y 2)"
                   "(ifa x y)"
                   "(set j 1)"
                   "(set x 2)"
                   "(set y 1)"
                   "(ifa x y)"
                   "(set a 1)"
                   "(set x 2)"
                   "(set y -1)"
                   "(ifa x y)"
                   "(set b 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetRegister(Registers_J), 0)
                   VerifyEqual(cpu.GetRegister(Registers_A), 1)
                   VerifyEqual(cpu.GetRegister(Registers_B), 1)
                   VerifyEqual(cpu.GetCycles(), 20)
                   );

    CreateTestCase("IFL",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifl x y)"
                   "(set i 1)"
                   "(set x 2)"
                   "(set y 2)"
                   "(ifl x y)"
                   "(set j 1)"
                   "(set x 2)"
                   "(set y 1)"
                   "(ifl x y)"
                   "(set a 1)"
                   "(set x -1)"
                   "(set y 2)"
                   "(ifl x y)"
                   "(set b 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetRegister(Registers_J), 0)
                   VerifyEqual(cpu.GetRegister(Registers_A), 0)
                   VerifyEqual(cpu.GetRegister(Registers_B), 0)
                   VerifyEqual(cpu.GetCycles(), 20)
                   );

    CreateTestCase("IFU",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifu x y)"
                   "(set i 1)"
                   "(set x 2)"
                   "(set y 2)"
                   "(ifu x y)"
                   "(set j 1)"
                   "(set x 2)"
                   "(set y 1)"
                   "(ifu x y)"
                   "(set a 1)"
                   "(set x -1)"
                   "(set y 2)"
                   "(ifu x y)"
                   "(set b 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetRegister(Registers_J), 0)
                   VerifyEqual(cpu.GetRegister(Registers_A), 0)
                   VerifyEqual(cpu.GetRegister(Registers_B), 1)
                   VerifyEqual(cpu.GetCycles(), 20)
                   );

    CreateTestCase("IfSeq",
                   "(set x 1)"
                   "(set y 2)"
                   "(ifg x y)"
                   "(ife x 1)"
                   "(ife y 2)"
                   "(set i 1)"
                   "(set a 1)"
                   "(set b 2)"
                   "(ifl a b)"
                   "(ife a 1)"
                   "(ife b 2)"
                   "(set j 1)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 0)
                   VerifyEqual(cpu.GetRegister(Registers_J), 1)
                   VerifyEqual(cpu.GetCycles(), 16)
                   );

    CreateTestCase("ADX",
                   "(set i 0xFFFF)"
                   "(adx i 2)"
                   "(adx j 3)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetRegister(Registers_J), 4)
                   VerifyEqual(cpu.GetCycles(), 7)
                   );

    CreateTestCase("SBX",
                   "(set i 1)"
                   "(sbx i 2)"
                   "(sbx j 3)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_I), static_cast<uint16_t>(-1))
                   VerifyEqual(cpu.GetRegister(Registers_J), static_cast<uint16_t>(-4))
                   VerifyEqual(cpu.GetCycles(), 7)
                   );

    CreateTestCase("STI",
                   "(set j 2)"
                   "(sti a 0xA)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_A), 0xA)
                   VerifyEqual(cpu.GetRegister(Registers_I), 1)
                   VerifyEqual(cpu.GetRegister(Registers_J), 3)
                   VerifyEqual(cpu.GetCycles(), 3)
                   );

    CreateTestCase("STD",
                   "(set j 2)"
                   "(std a 0xA)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_A), 0xA)
                   VerifyEqual(cpu.GetRegister(Registers_I), 0xFFFF)
                   VerifyEqual(cpu.GetRegister(Registers_J), 1)
                   VerifyEqual(cpu.GetCycles(), 3)
                   );

    CreateTestCase("LABELS",
                   "(set x 1)"
                   "(set y 3)"
                   "(label loop)"
                   "(ife y 0)"
                   "(set pc done)"
                   "(mul x 3)"
                   "(sub y 1)"
                   "(set pc loop)"
                   "(label done)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 27)
                   VerifyEqual(cpu.GetRegister(Registers_Y), 0)
                   VerifyEqual(cpu.GetCycles(), 33)
                   );

    CreateTestCase("JSR",
                   "(set x 5)"
                   "(jsr sqr)"
                   "(jsr sqr)"
                   "(set push 11)"
                   "(jsr addten)"
                   "(set y pop)"
                   "(set pc done)"
                   "(label sqr)"
                   "(mul x x)"
                   "(set pc pop)"
                   "(label addten)"
                   "(add (ref sp 1) 10)"
                   "(set pc pop)"
                   "(label done)"
                   ,
                   VerifyEqual(cpu.GetRegister(Registers_X), 625)
                   VerifyEqual(cpu.GetRegister(Registers_Y), 21)
                   VerifyEqual(cpu.GetCycles(), 27)
                   );

    if (!shouldStop)
        printf("All Tests Completed Successfully\n");
    return 0;
}
