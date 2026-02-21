#include "FunctionCall.hpp"
#include "types/type.h"
#include "ReturnStatement.hpp"

uref<ReturnStatement> ReturnStatement::parse(Lexer& lexer) {
    lexer.savePosition();
    auto ret = create_unique<ReturnStatement>();

    if (!lexer.expectToken(RETURN)) {
        ret->valid = false;
        ret->lastToken = lexer.nextToken();
        ret->expected = tokenKindsToString({RETURN});
        lexer.rollPosition();
        return ret;
    }

    auto expr = Expression::parse(lexer);
    if (!expr->valid) {
        ret->valid = false;
        ret->lastToken = expr->lastToken;
        ret->expected = expr->expected;
        lexer.rollPosition();
        return ret;
    }

    if (!lexer.expectToken(SEMICOLON)) {
        ret->valid = false;
        ret->expected = tokenKindsToString({SEMICOLON});
        ret->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return ret;
    }

    ret->expression = move(expr);
    ret->valid = true;
    lexer.deletePosition();
    return ret;
}


Value ReturnStatement::execute(Scope& scope) {
    auto value = expression->execute(scope);
    value.returning = true;
    return value;
}
