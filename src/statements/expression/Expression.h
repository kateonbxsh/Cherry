#pragma once

#include "statement.h"
#include "types/type.h"
#include "lexer.h"
#include "scope.h"

class Expression : public Statement {
public:

    static Expression* parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Statement* firstOperand = nullptr;
    Statement* secondOperand = nullptr;
    Token expressionOperator;
};

class ExpressionParenWrapped : public Statement {

public:

    static ExpressionParenWrapped* parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Expression* child = nullptr;

};

class ExpressionValue : public Statement {

public:

    static ExpressionValue* parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Token identifier;

};