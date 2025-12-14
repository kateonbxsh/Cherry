#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/Block.hpp"
#include "statements/expression/Expression.h"
#include "statements/function/FunctionCall.hpp"
#include <vector>

struct IfClause {
    uref<Expression> condition;
    uref<Block> body;
    bool isUnless = false; // per clause
};

class IfStatement : public Statement {
public:

    static uref<IfStatement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    std::vector<IfClause> clauses; // 'if' and 'else if/unless'
    uref<Block> elseClause;        // optional 'else'
};
