#pragma once

#include <stack>
#include "lexer.h"
#include "scope.h"
#include "statement.h"

class Parser {

public:

    explicit Parser(Lexer& inputLexer);
    Statement* parse();

private:

    Lexer lexer;

};