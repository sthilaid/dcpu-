#include <dcpu-types.h>
#include <dcpu-assert.h>
#include <dcpu-tokenizer.h>

bool is_num(char c) { return c >= '0' && c <= '9'; }
bool is_newline(char c) { return c == '\n'; }
bool is_seperator(char c) {
    switch (c) {
    case ' ':
    case '\t':
        return true;
    default:
        return false;
    }
}

Token::Token(char c)
    : Type(c == '(' ? LParen : RParen)
    , NumVal(0)
    , SymVal("") {
    dcpu_assert(c == '(' || c == ')', "Expecting Parentesis");
}

Token::Token(const string& str)
    : Type(Symbol)
    , NumVal(0)
    , SymVal(str) {
    dcpu_assert(!str.empty(), "Expecting non empty string");

    int baseStart = 0;
    if ((str[0] == '+' || str[0] == '-') && str.size() > 1)
        baseStart = 1;
        
    if (is_num(str[baseStart])) {
        Type = Number;
        int base = 10;
        if (str.size() >= baseStart+3 && str[baseStart] == '0' && str[baseStart+1] == 'x')
            base = 16;
        else if (str.size() >= baseStart+2 && str[baseStart] == '0')
            base = 8;
        NumVal = std::stoi(str, nullptr, base);
    }
}

vector<Token> Token::Tokenize(std::basic_istream<char>& inputStream) {
    vector<Token> tokens;
    string current;
    char c;
    bool is_commenting = false;
    while (true) {
        inputStream.read(&c, 1);
        if (inputStream.eof())
            break; // need to check eof after the read call
        
        if (is_newline(c)) {
            is_commenting = false;
        } else if (is_commenting || c == ';') {
            is_commenting = true;
        } else if (c == '(' || c == ')') {
            if (current != "") {
                tokens.push_back(Token{current});
                current = "";
            }
            tokens.push_back(Token{c});
        } else if (is_seperator(c)) {
            if (current != "") {
                tokens.push_back(Token{current});
                current = "";
            }
        } else {
            current += c;
        }
    } 
    return tokens;
}

