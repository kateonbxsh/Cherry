#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"

class VariableAffectation : public Statement {

public:

    static VariableAffectation* parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Expression* expression;
    Token name;
    TokenKind selfOperation;

};
