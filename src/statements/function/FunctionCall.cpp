#include "FunctionCall.hpp"
#include "types/type.h"
#include "types/function.h"
#include "LambdaExpression.hpp"
#include <expressions.h>
#include "runtime_builtins.h"

static Value makeCallError(const std::string& message) {
    Value v;
    v.thrownException = create_reference<Value>(Value(message));
    return v;
}

static bool bindCallArguments(
    const Function& function,
    const std::vector<Value>& args,
    std::vector<Value>& bound,
    std::string& error
) {
    if (!function.validArguments(args)) {
        error = "arguments are not compatible with function signature";
        return false;
    }

    size_t fixedCount = function.parameters.size();
    bool variadic = !function.parameters.empty() && function.parameters.back().variadic;
    if (variadic) fixedCount--;

    bound.clear();
    bound.reserve(function.parameters.size());
    for (size_t i = 0; i < fixedCount; ++i) bound.push_back(args[i]);

    if (variadic) {
        std::vector<Value> tail;
        for (size_t i = fixedCount; i < args.size(); ++i) {
            tail.push_back(args[i]);
        }
        Value packed = makeArrayFromValues(tail);
        if (packed.thrownException != nullptr) {
            error = stringify(*packed.thrownException);
            return false;
        }
        bound.push_back(packed);
    }

    return true;
}

uref<FunctionCall> FunctionCall::parse(Lexer& lexer) {
    return create_unique<FunctionCall>();
}


Value FunctionCall::execute(Scope& scope) {

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering function call" << std::endl;
    auto func = function->execute(scope);
    if (DEBUG) std::cout << DEBUG_PREFIX << "Got function value: " << stringify(func) << std::endl;

    if (func.thrownException != nullptr) return func;

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

    std::vector<Value> boundArgs;
    std::string bindError;
    if (!bindCallArguments(function, argValues, boundArgs, bindError)) {
        return makeCallError(bindError);
    }

    if (function.kind == FunctionKind::Internal) {
        if (!function.internalHandler) {
            return makeCallError("internal function is missing implementation");
        }
        return function.internalHandler(scope, boundArgs, function.__this);
    }

    // Execute function body in a new scope
    Scope funcScope(function.closure);
    if (function.__this != nullptr) {
        funcScope.addVariable("this", *function.__this);
    }
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        funcScope.addVariable(function.parameters[i].name, boundArgs[i]);
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering main block" << stringify(func) << std::endl;
    return function.body->execute(funcScope);
}
