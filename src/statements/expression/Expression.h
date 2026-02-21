#pragma once

#include "statement.h"
#include "types/type.h"
#include "lexer.h"
#include "scope.h"
#include <vector>

class Expression : public Statement {
public:

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    uref<Expression> firstOperand = nullptr;
    uref<Expression> secondOperand = nullptr;
    Token expressionOperator;

    bool conditional;
    bool unlessCondition;
    bool withElseValue;
    uref<Expression> condition;
    uref<Expression> elseValue;

};

class ExpressionParenWrapped : public Statement {

public:

    static uref<Expression> parse(Lexer& lexer);

};

class ExpressionValue : public Expression {

public:

    static uref<ExpressionValue> parse(Lexer& lexer);
    Value execute(Scope& scope) override;
    bool isPlainIdentifier(Token* outIdentifier = nullptr) const;

private:

    Token identifier;
    std::vector<Token> typeArguments;
    bool hasTypeArguments = false;

};

class UnaryExpression : public Expression {

public:

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    uref<Expression> expression;

};

class DotAccessExpression : public Expression {
public:

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    uref<Expression> target;
    Token member;

};

class IndexAccessExpression : public Expression {
public:
    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    uref<Expression> target;
    std::vector<uref<Expression>> arguments;
};

class ConstructorExpression : public Expression {
public:

    static uref<Expression> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    Token typeName;
    std::vector<Token> typeArguments;
    std::vector<uref<Expression>> arguments;

};
