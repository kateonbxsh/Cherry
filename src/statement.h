#pragma once

#include "lexer.h"
#include "data.h"
#include "scope.h"

class Statement {
public:

    virtual Value execute(Scope& scope) = 0;

    bool valid = true;
    Token lastToken;
    std::vector<string> expected;
    std::string errorMessage;

    void invalidExpected(std::initializer_list<TokenKind> kinds, Lexer& lex);
    void invalidExpected(const std::vector<TokenKind>& kinds, Lexer& lex);
    void invalidFrom(const Statement& other);

};

class NotAStatement : public Statement {
public:
    Value execute(Scope& scope);
};

uref<Statement> getFurthestInvalidStatement(const std::vector<uref<Statement>>& statements);
