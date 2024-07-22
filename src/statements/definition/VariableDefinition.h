#pragma once

#include "../../statement.h"
#include "../../lexer.h"
#include "../expression/Expression.h"

class VariableDefinition : public Statement {

public:

    static VariableDefinition* parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    Expression* expression = nullptr;
    Token type;
    Token name;

};
