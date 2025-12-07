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

};

class NotAStatement : public Statement {
public:
    Value execute(Scope& scope);
};

uref<Statement> getFurthestInvalidStatement(const std::vector<uref<Statement>>& statements);
