#include "types/type.h"
#include "ExpressionStatement.hpp"

uref<ExpressionStatement> ExpressionStatement::parse(Lexer& lexer) {
    
    lexer.savePosition();
    auto funcStatement = create_unique<ExpressionStatement>();

    auto call = Expression::parse(lexer);
    if (!call->valid) {
        funcStatement->valid = false;
        funcStatement->expected = call->expected;
        funcStatement->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return funcStatement;
    }

    if (!lexer.expectToken(SEMICOLON)) {
        funcStatement->valid = false;
        funcStatement->expected = tokenKindsToString({SEMICOLON});
        funcStatement->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return funcStatement;
    }
    funcStatement->expression = move(call);
    funcStatement->valid = true;
    lexer.deletePosition();
    return funcStatement;
}

Value ExpressionStatement::execute(Scope& scope) {
    return expression->execute(scope);
}
