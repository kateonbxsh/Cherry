#pragma once

#include "../../statement.h"
#include "../../lexer.h"
#include "../expression/Expression.h"

class VariableDefinition : public Statement {

public:

    static VariableDefinition* parse(Lexer& lexer);
    void execute() override;

private:

    Expression* expression = nullptr;
    Token type;
    Token name;

};
