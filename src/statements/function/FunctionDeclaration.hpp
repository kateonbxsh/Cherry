#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/Block.hpp"
#include <vector>

typedef struct {
    Token name;
    Token type;
} Parameter;

class FunctionDeclaration : public Statement {
public:

    static uref<FunctionDeclaration> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    Token name;
    std::vector<Parameter> parameters;
    reference<Block> body;
};
