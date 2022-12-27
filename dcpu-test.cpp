#include <cassert>
#include <dcpu.h>
#include <dcpu-lispasm.h>
#include <decoder.h>
#include <sstream>

const char* Test1 =
    "(set X 12)\n";

const char* Test2 =
    "(set X 12)\n"
    "(set (ref x) 21)";

const char* Test3 =
    "(set push 14)\n"
    "(add peek 1)"
    "(set b 0x7)"
    "(and b pop)"
    "(set a (ref sp -1))"
    ;

// #define CreateTestCase(source, ...) {           \
//     TestCase t(source); \
//     t.AddVerifier
#define TestCaseAddVerif(test, verif) test.AddVerifier([](const DCPU& cpu, const Memory& mem) { return verif; }, #verif);

class TestCase {
public:
    using VerifyType = bool(*)(const DCPU& cpu, const Memory& mem);

    string m_lasmSource = "";
    vector<VerifyType> m_verifiers;
    vector<const char*> m_verifiersTxt;
    int m_testid = 0;
    static int s_id;

    TestCase(string source) : m_lasmSource(source), m_verifiers(), m_verifiersTxt(), m_testid(s_id++) {}
    void AddVerifier(VerifyType v, const char* vStr) {
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

    bool success = true;
    DCPU cpu;
    Memory mem;
    cpu.Run(mem, codebytes);
    for (int i=0; i < m_verifiers.size(); ++i) {
        success &= m_verifiers[i](cpu, mem);
        if (success)
            printf("Test %d-%d [SUCCESS] : %s\n", m_testid, i, m_verifiersTxt[i]);
        else {
        failedtest:
            printf("Test %d-%d [FAILURE] : %s\n", m_testid, i, m_verifiersTxt[i]);
        }
    }

    return success;
}

int main(int argc, char** argv) {
    TestCase test1{Test1};
    TestCaseAddVerif(test1, cpu.GetRegister(Registers_X) == 12);
    test1.TryTest();

    TestCase test2{Test2};
    TestCaseAddVerif(test2, mem[12] == 21);
    test2.TryTest();

    TestCase test3{Test3};
    TestCaseAddVerif(test3, cpu.GetSP() == 0xFFFF);
    TestCaseAddVerif(test3, cpu.GetRegister(Registers_B) == 7);
    TestCaseAddVerif(test3, mem[0xFFFE] == 15);
    TestCaseAddVerif(test3, cpu.GetRegister(Registers_A) == 15;)
    test3.TryTest();
}
