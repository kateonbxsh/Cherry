#pragma once

#include "lexer.h"
#include "statement.h"
#include "scope.h"
#include <data.h>

class GlobalBlock : public Statement {

public:

    static uref<GlobalBlock> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    std::vector<uref<Statement>> children = {};

};