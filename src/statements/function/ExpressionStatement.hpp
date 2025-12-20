#include "statement.h"
#include "statements/expression/Expression.h"

class ExpressionStatement : public Statement {
public:

    static uref<ExpressionStatement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    uref<Expression> expression;
};
