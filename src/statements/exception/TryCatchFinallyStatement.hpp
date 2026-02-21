#pragma once

#include "statement.h"
#include "statements/Block.hpp"

struct CatchClause {
    Token typeName;
    Token variableName;
    uref<Block> body;
};

class TryCatchFinallyStatement : public Statement {
public:
    static uref<TryCatchFinallyStatement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    Token tryToken;
    uref<Block> tryBlock;
    std::vector<CatchClause> catches;
    uref<Block> finallyBlock;
};

