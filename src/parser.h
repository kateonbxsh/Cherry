#pragma once

#include <stack>
#include <lexer.h>
#include <scope.h>
#include <statement.h>
#include "statements/GlobalBlock.h"

class Parser {

public:

    explicit Parser(Lexer& inputLexer);
    uref<GlobalBlock> parse();

private:

    Lexer lexer;

};