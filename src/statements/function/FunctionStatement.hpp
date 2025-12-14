#include "statement.h"
#include "FunctionCall.hpp"

class FunctionStatement : public Statement {
public:

    static uref<FunctionStatement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    uref<FunctionCall> call;
};
