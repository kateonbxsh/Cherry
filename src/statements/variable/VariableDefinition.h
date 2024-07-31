#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"

class VariableDefinition : public Statement {

public:

    static unique<VariableDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    bool inferred = false;
    unique<Expression> expression;
    Token type;
    Token name;

};
