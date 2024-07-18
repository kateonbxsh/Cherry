#pragma once

#include "../../statement.h"
#include "../../type.h"
#include "../../lexer.h"

class VariableDefinition : public Statement {

public:

    static VariableDefinition* parse(Lexer& lexer);
    void execute() override;

private:

    Expression* expression;
    Token type;
    Token name;

};
