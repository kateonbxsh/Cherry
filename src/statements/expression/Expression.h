#pragma once

#include "statement.h"
#include "types/type.h"
#include "lexer.h"
#include "scope.h"

class Expression : public Statement {
public:

    static unique<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    unique<Expression> firstOperand = nullptr;
    unique<Expression> secondOperand = nullptr;
    Token expressionOperator;
};

class ExpressionParenWrapped : public Statement {

public:

    static unique<Expression> parse(Lexer& lexer);

};

class ExpressionValue : public Expression {

public:

    static unique<ExpressionValue> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Token identifier;

};