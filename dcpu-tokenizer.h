#pragma once
#include <istream>
#include <vector>
#include <string>

using std::string;
using std::vector;

struct Token {
    enum TokenType {
        LParen,
        RParen,
        Symbol,
        Number,
    };

    TokenType Type = LParen;
    uint16_t NumVal = 0;
    string SymVal = "";

    Token();
    Token(char c);
    Token(const string& str);

    static vector<Token> Tokenize(std::basic_istream<char>& inputStream);
};


