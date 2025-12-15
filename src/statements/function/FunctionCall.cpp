#include "FunctionCall.hpp"
#include "types/type.h"
#include "FunctionDeclaration.hpp"

uref<FunctionCall> FunctionCall::parse(Lexer& lexer) {
    lexer.savePosition();
    auto call = create_unique<FunctionCall>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering FunctionCall::parse\n";

    // Expect function name (identifier)
    Token next = lexer.nextToken();
    if (next.kind != IDENTIFIER) {
        if (DEBUG) {
            std::cout << DEBUG_WARNING_PREFIX
                      << "FunctionCall failed: expected IDENTIFIER, "
                      << "Found: " << next.value << "\n";
        }

        call->valid = false;
        call->expected = {"function name"};
        call->lastToken = next;
        lexer.rollPosition();
        return call;
    }

    call->functionName = next;

    if (DEBUG) {
        std::cout << DEBUG_PREFIX
                  << "Function name parsed: " << next.value << "\n";
    }

    // Expect "("
    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected '(' after function name\n";

        call->valid = false;
        call->expected = tokenKindsToString({LEFT_PARENTHESIS});
        call->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return call;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing function arguments\n";

    // Parse arguments (expressions separated by commas)
    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        auto arg = Expression::parse(lexer);
        if (!arg->valid) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid expression in function arguments\n";

            call->valid = false;
            call->expected = arg->expected;
            call->lastToken = arg->lastToken;
            lexer.rollPosition();
            return call;
        }

        call->arguments.push_back(move(arg));

        if (DEBUG) {
            std::cout << DEBUG_SUCCESS_PREFIX
                      << "Parsed argument #" << call->arguments.size() << "\n";
        }

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) {
            if (DEBUG) std::cout << DEBUG_PREFIX << "End of argument list\n";
            break;
        }

        if (sep.kind != COMMA) {
            if (DEBUG) {
                std::cout << DEBUG_ERROR_PREFIX
                          << "Expected ',' or ')', got\n";
            }

            call->valid = false;
            call->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            call->lastToken = sep;
            lexer.rollPosition();
            return call;
        }

        if (DEBUG) std::cout << DEBUG_PREFIX << "Comma found, parsing next argument\n";
    }

    call->valid = true;

    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "FunctionCall parsed successfully\n";
    lexer.deletePosition();

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
