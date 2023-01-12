#include <dcpu-lispasm.h>
#include <dcpu-tokenizer.h>
#include <fstream>

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu-asm-test <source-file>\n");
        return 1;
    }

    std::ifstream inputStream(args[1], std::ios::in);
    
    if (!inputStream.is_open()) {
        printf("unknown file: %s\n", args[1]);
        return 1;
    }
    vector<Token> tokens = Token::Tokenize(inputStream);
    printf("tokens count: %d\n", tokens.size());

    vector<SExp*> expressions = SExp::FromTokens(tokens);
    for (const SExp* sexp : expressions) {
        printf("sexp: %s\n", sexp->toStr().c_str());
    }

    vector<Instruction> instructions = LispAsmParser::FromSExpressions(expressions);
    for (const Instruction& i : instructions) {
        printf("parsed: %s\n", i.toStr().c_str());
    }
    SExp::Delete(expressions);

    return 0;
}
