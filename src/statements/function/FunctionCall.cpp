#include "FunctionCall.hpp"
#include "types/type.h"
#include "types/function.h"
#include "FunctionDefinition.hpp"
#include <expressions.h>

uref<FunctionCall> FunctionCall::parse(Lexer& lexer) {
    return create_unique<FunctionCall>();
}


Value FunctionCall::execute(Scope& scope) {

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering function call" << std::endl;
    auto func = function->execute(scope);
    if (DEBUG) std::cout << DEBUG_PREFIX << "Got function value: " << stringify(func) << std::endl;

    if (func.type != FunctionType) {
        auto exception = create_reference<Value>(Value("value is not a function"));
        Value v;
        v.thrownException = exception;
        return v;
    }

    auto function = get<Function>(func.value);

    if (DEBUG) std::cout << DEBUG_PREFIX << "Evaluating arguments" << stringify(func) << std::endl;
    // Evaluate arguments
    std::vector<Value> argValues;
    for (auto& arg : arguments) {
        Value val = arg->execute(scope);
        if (val.thrownException != nullptr) return val;
        argValues.push_back(val);
    }
    if (DEBUG) std::cout << DEBUG_PREFIX << "Done evaluating arguments" << stringify(func) << std::endl;

    // Execute function body in a new scope
    Scope funcScope(function.closure); // child scope
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        auto expectedType = function.parameters[i].type;
        if (argValues[i].type != expectedType) {
            Value exception;
            exception.thrownException = create_reference<Value>(
                Value("function argument type expected: " + expectedType->getName() + ", got: " + argValues[i].type->getName())
            );
            return exception;
        }
        funcScope.addVariable(function.parameters[i].name, argValues[i]);
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering main block" << stringify(func) << std::endl;
    return function.body->execute(funcScope);
}
