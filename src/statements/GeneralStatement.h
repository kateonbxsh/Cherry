#pragma once

#include "lexer.h"
#include "statement.h"
#include "scope.h"
#include <data.h>

class GeneralStatement : public Statement {

public:

    static uref<Statement> parse(Lexer& lexer);

};