#include "../statement.h"
#include "../type.h"
#include "../lexer.h"

class IfStatement : public Statement {

public:
    IfStatement(): Statement(IF_STATEMENT) {};

    class SubIfStatement {
    public:
        bool isUnless = false;
        bool isElse = false;
        bool elseHasExpression = false;
        TokenList expression;
        Block block;

    };

    std::vector<SubIfStatement*> subStatements;

};
