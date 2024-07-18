#pragma once

#include "lexer.h"

class Statement {

public:

    static Statement* parse(Lexer& lexer);
    virtual void execute() {};

    bool valid = true;
    Token lastToken;
    std::vector<TokenKind> expected;

};

Statement* getFurthestInvalidStatement(std::vector<Statement*> statements);

void deleteAllStatements(std::vector<Statement*> statements);