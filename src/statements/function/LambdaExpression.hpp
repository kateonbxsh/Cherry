#pragma once
#include "../expression/Expression.h"
#include "../Block.hpp"

struct MethodParameter {
    Token type;
    Token name;
};

struct MethodDefinition : public Expression {
    std::vector<MethodParameter> parameters;
    reference<Block> body;

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;
};
