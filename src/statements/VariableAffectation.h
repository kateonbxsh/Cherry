#include "../statement.h"
#include "../type.h"
#include "../lexer.h"

class VariableAffectation : public Statement {

public:
    VariableAffectation(): Statement(VARIABLE_AFFECTATION) {};

    std::string name;
    TokenList expression;

};
