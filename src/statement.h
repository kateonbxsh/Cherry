#pragma once

#include "lexer.h"
#include "data.h"
#include "scope.h"

class Statement {

public:

    virtual Value execute(Scope& scope) = 0;

    bool valid = true;
    Token lastToken;
    string expected;

};

unique<Statement> getFurthestInvalidStatement(const std::vector<unique<Statement>>& statements);
