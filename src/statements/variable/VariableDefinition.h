#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"
#include "type_syntax.h"

class VariableDefinition : public Statement {

public:

    static uref<VariableDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    bool inferred = false;
    uref<Expression> expression;
    TypeSyntaxExpression type;
    Token name;

};
