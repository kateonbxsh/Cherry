#pragma once

#include "lexer.h"

class Statement {

public:

    static Statement* parse(Lexer& lexer);
    virtual void execute() {};

protected:

    bool valid = true;
    Token lastToken;
    std::vector<TokenKind> expected;

};

class NotAStatement : public Statement {

public:

    NotAStatement(Token lastToken, std::vector<TokenKind> expected);

private:

    bool valid = false;

};