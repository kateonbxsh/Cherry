#pragma once

#include <stack>
#include "lexer.h"
#include "scope.h"
#include "statement.h"

typedef std::stack<std::string> Callstack;

enum CompilerState {

    GLOBAL,

    RET_VAR_NAME,
    RET_VAR_EQUAL,

    AWAITING_SEMICOLON,

    AWAITING_BLOCK,
    RET_ELSE_IF,
    RET_ELSE,

};

enum LexicalStack {

    BRACKET,
    BRACE

};

class Compiler {

public:
    explicit Compiler(Lexer& inputLexer);
    Block parse();

private:

    Block parseInternal(Scope scope);
    TokenList parseExpression();
    Lexer lexer;
    Callstack callstack;
    std::stack<LexicalStack> lexicalStack;

};