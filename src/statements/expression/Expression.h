#pragma once

#include "../../statement.h"
#include "../../type.h"
#include "../../lexer.h"

class Expression : public Statement {
public:

    static Expression* parse(Lexer& lexer);
    void execute() override;

private:

    Statement* firstOperand;
    Statement* secondOperand;
    Token expressionOperator;
};

class ExpressionParenWrapped : public Statement {

    static ExpressionParenWrapped* parse(Lexer& lexer);
    void execute() override;

private:

    Expression* child;

};
