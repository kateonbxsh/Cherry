#include "FunctionCall.hpp"
#include "types/type.h"
#include "FunctionStatement.hpp"

uref<FunctionStatement> FunctionStatement::parse(Lexer& lexer) {
    
    lexer.savePosition();
    auto funcStatement = create_unique<FunctionStatement>();

    auto call = FunctionCall::parse(lexer);
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
    funcStatement->call = move(call);
    funcStatement->valid = true;
    lexer.deletePosition();
    return funcStatement;
}

Value FunctionStatement::execute(Scope& scope) {
    return call->execute(scope);
}
