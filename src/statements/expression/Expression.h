#pragma once

#include "statement.h"
#include "types/type.h"
#include "lexer.h"
#include "scope.h"

class Expression : public Statement {
public:

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    uref<Expression> firstOperand = nullptr;
    uref<Expression> secondOperand = nullptr;
    Token expressionOperator;
};

class ExpressionParenWrapped : public Statement {

public:

    static uref<Expression> parse(Lexer& lexer);

};

class ExpressionValue : public Expression {

public:

    static uref<ExpressionValue> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Token identifier;

};