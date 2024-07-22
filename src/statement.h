#pragma once

#include "lexer.h"

class Statement {

public:

    virtual Value execute(Scope& scope) {};

    bool valid = true;
    Token lastToken;
    std::vector<TokenKind> expected;

};

Statement* getFurthestInvalidStatement(const std::vector<Statement*>& statements);

void deleteAllStatements(const std::vector<Statement*>& statements);