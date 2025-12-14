#pragma once

#include "statement.h"
#include "lexer.h"
#include "GeneralStatement.h"
#include <vector>

class Block : public Statement {
public:

    static uref<Block> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    std::vector<uref<Statement>> statements;
};
