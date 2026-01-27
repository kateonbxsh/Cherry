#pragma once
#include "../expression/Expression.h"
#include "../Block.hpp"

struct LambdaParameter {
    Token type;
    Token name;
};

struct LambdaExpression : public Expression {
    std::vector<LambdaParameter> parameters;
    reference<Block> body;

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;
};
