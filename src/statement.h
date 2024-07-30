#pragma once

#include "lexer.h"
#include "data.h"
#include "scope.h"

class Statement {

public:

    virtual Value execute(Scope& scope) = 0;

    bool valid = true;
    Token lastToken;
    std::string expected;

};

Statement* getFurthestInvalidStatement(const std::vector<Statement*>& statements);

void deleteAllStatements(std::vector<Statement*>& statements);