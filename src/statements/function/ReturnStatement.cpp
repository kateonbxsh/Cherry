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
        return ret;
    }

    auto expr = Expression::parse(lexer);
    if (!expr->valid) {
        ret->valid = false;
        ret->lastToken = expr->lastToken;
        ret->expected = expr->expected;
        return ret;
    }

    ret->expression = move(expr);
    ret->valid = true;
    lexer.deletePosition();
    return ret;
}


Value ReturnStatement::execute(Scope& scope) {

    return NullValue;
}
