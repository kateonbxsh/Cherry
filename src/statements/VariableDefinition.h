#include "../statement.h"
#include "../type.h"
#include "../lexer.h"

class VariableDefinition : public Statement {

public:
    VariableDefinition(): Statement(VARIABLE_DEFINITION) {};

    std::string type;
    std::string name;
    TokenList expression;
};
