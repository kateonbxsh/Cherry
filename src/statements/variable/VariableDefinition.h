#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"

class VariableDefinition : public Statement {

public:

    static VariableDefinition* parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    bool inferred = false;
    Expression* expression;
    Token type;
    Token name;

};
