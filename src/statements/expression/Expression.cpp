#include "Expression.h"

Expression *Expression::parse(Lexer &lexer) {
    lexer.savePosition();

    Token varType, varName, varExpression;

    auto nextToken = lexer.nextToken();
    if (nextToken.kind == IDENTIFIER) {
        varType = nextToken;
    } else {
        lexer.rollPosition();
        return new NotAStatement(nextToken, {IDENTIFIER});
    }

    nextToken = lexer.nextToken();
    if (nextToken.kind == IDENTIFIER) {
        varName = nextToken;
    } else {
        lexer.rollPosition();
        return new NotAStatement(nextToken, {IDENTIFIER});
    }




}

void Expression::execute() {
    Statement::execute();
}
