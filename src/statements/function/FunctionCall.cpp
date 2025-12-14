#include "FunctionCall.hpp"
#include "types/type.h"
#include "FunctionDeclaration.hpp"

uref<FunctionCall> FunctionCall::parse(Lexer& lexer) {
    lexer.savePosition();
    auto call = create_unique<FunctionCall>();

    // Expect function name (identifier)
    Token next = lexer.nextToken();
    if (next.kind != IDENTIFIER) {
        call->valid = false;
        call->expected = {"function name"};
        call->lastToken = next;
        lexer.rollPosition();
        return call;
    }
    call->functionName = next;

    // Expect "("
    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        call->valid = false;
        call->expected = tokenKindsToString({LEFT_PARENTHESIS});
        call->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return call;
    }

    // Parse arguments (expressions separated by commas)
    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        auto arg = Expression::parse(lexer);
        if (!arg->valid) {
            call->valid = false;
            call->expected = arg->expected;
            call->lastToken = arg->lastToken;
            lexer.rollPosition();
            return call;
        }

        call->arguments.push_back(move(arg));

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) break;
        if (sep.kind != COMMA) {
            call->valid = false;
            call->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            call->lastToken = sep;
            lexer.rollPosition();
            return call;
        }
    }

    call->valid = true;
    return call;
}

Value FunctionCall::execute(Scope& scope) {

    // Retrieve the function from the scope
    /*auto func = scope.getFunction(functionName.value);
    if (!func) {
        Value err;
        err.thrownException = new std::string("Function not found: " + functionName.value);
        return err;
    }

    // Evaluate arguments
    std::vector<Value> argValues;
    for (auto& arg : arguments) {
        Value val = arg->execute(scope);
        if (val.thrownException != nullptr) return val;
        argValues.push_back(val);
    }

    // Execute function body in a new scope
    Scope funcScope(&scope); // child scope
    for (size_t i = 0; i < func->parameters.size(); ++i) {
        funcScope.setVariable(func->parameters[i].value, argValues[i]);
    }

    return func->body->execute(funcScope);*/
    return NullValue;
}
