#include <dcpu-lisp.h>
#include <dcpu-assert.h>

vector<Instruction> LispParser::FromSExpressions(const vector<SExp*>& sexpressions) {
    vector<Instruction> instructions;
    for (const SExp* exp : sexpressions) {
        if (exp->isSpecialForm("lambda")) {
            dcpu_assert_fmt(exp->m_values.size() > 1, "lambda expression expecting argument list but foudn none: %s",
                            exp->toStr().c_str());
            dcpu_assert_fmt(std::all_of(exp->m_values.begin(), exp->m_values.end(),
                                        [](const SExp::Val& v) { return v.m_type == SExp::Val::Symbol; }),
                            "lambda expression only expecting symbol arguments: %s",
                            exp->toStr().c_str());

                        
        }
    }
    return instructions;
}
