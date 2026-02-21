#pragma once

#include <stack>
#include <lexer.h>
#include <scope.h>
#include <statement.h>
#include "statements/GlobalBlock.h"

class Parser {

public:

    Parser(Lexer& inputLexer, std::string filePath);
    uref<GlobalBlock> parse();

private:

    Lexer lexer;
    std::string filePath;

};
