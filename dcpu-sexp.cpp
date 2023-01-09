#include <dcpu-sexp.h>
#include <dcpu-tokenizer.h>
#include <dcpu-assert.h>

string SExp::Val::toStr() const {
    switch (m_type) {
    case Number: return std::to_string(m_numVal);
    case Symbol: return m_symVal;
    case SExp: return m_sexpVal->toStr();
    default:
        dcpu_assert(false, "unhandled value type");
        return nullptr;
    }
}

void SExp::Delete(SExp* sexp) {
    for (Val& v : sexp->m_values) {
        if (v.m_type == Val::SExp) {
            Delete(v.m_sexpVal);
        }
    }
    delete sexp;
}

void SExp::Delete(vector<SExp*>& expressions){
    for (SExp* exp : expressions) {
        SExp::Delete(exp);
    }
    expressions.clear();
}

vector<SExp*> SExp::FromTokens(const vector<Token>& tokens) {
    vector<SExp*> expressions;
    word_t i=0;
    while (i<tokens.size()) {
        expressions.push_back(FromTokens(tokens, i));
    }
    return expressions;
}

SExp* SExp::FromTokens(const vector<Token>& tokens, word_t& i) {
    dcpu_assert_fmt(tokens.size()-i >= 2, "Expected more tokens. Left: %d, expecting 2", (tokens.size()-i));
    dcpu_assert_fmt(tokens[i].Type == Token::LParen, "Expecting LParen as first sexp token, got %d", tokens[i].Type);
    ++i;
        
    SExp* current = new SExp{};
    while (i<tokens.size()) {
        switch (tokens[i].Type) {
        case Token::LParen: {
            Val subExp{FromTokens(tokens, i)};
            current->m_values.push_back(subExp);
            break;
        }
        case Token::RParen: {
            ++i;
            return current;
        }
        case Token::Number: {
            Val n{tokens[i++].NumVal};
            current->m_values.push_back(n);
            break;
        }
        case Token::Symbol: {
            Val s{tokens[i++].SymVal};
            current->m_values.push_back(s);
            break;
        }
        }
    }
    dcpu_assert(false, "Missing tokens to finish sexp");
    return nullptr;
}

string SExp::toStr() const {
    string str = "(";
    for (int i=0; i<m_values.size(); ++i) {
        str += m_values[i].toStr();
        str += i<m_values.size()-1 ? " " : ")";
    }
    return str;
}
