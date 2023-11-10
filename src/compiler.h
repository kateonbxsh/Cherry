#pragma once

#include <stack>
#include "lexer.h"
#include "scope.h"
#include "statement.h"

enum CompilerState {

    GLOBAL,

    RET_VAR_NAME,
    RET_VAR_EQUAL,

    AWAITING_SEMICOLON,

    AWAITING_BLOCK,
    RET_ELSE,

};


class Compiler {

public:
    explicit Compiler(Lexer& inputLexer);
    Block parse();

private:

    Block parseInternal(Scope scope);
    TokenList parseExpression();
    Lexer lexer;

};