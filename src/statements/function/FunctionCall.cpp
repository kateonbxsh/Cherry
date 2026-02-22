#include "FunctionCall.hpp"
#include "types/type.h"
#include "types/function.h"
#include "LambdaExpression.hpp"
#include <expressions.h>
#include "runtime_builtins.h"
#include "runtime_exception.h"

static std::string valueTypeName(const Value& value) {
    if (value.type == nullptr) return "null";
    return value.type->getName();
}

static std::string expectedSignature(const Function& function) {
    std::string out = "(";
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        const auto& p = function.parameters[i];
        if (i > 0) out += ", ";
        out += (p.type ? p.type->getName() : std::string("unknown"));
        if (p.variadic) out += "...";
        out += " ";
        out += p.name;
    }
    out += ")";
    return out;
}

static std::string describeArgumentMismatch(const Function& function, const std::vector<Value>& args) {
    size_t fixedCount = function.parameters.size();
    bool variadic = !function.parameters.empty() && function.parameters.back().variadic;
    if (variadic) fixedCount--;

    if (!variadic && args.size() != fixedCount) {
        return "arguments are not compatible with function signature: expected " + std::to_string(fixedCount) +
               ", got " + std::to_string(args.size()) + " for " + expectedSignature(function);
    }
    if (variadic && args.size() < fixedCount) {
        return "arguments are not compatible with function signature: expected at least " + std::to_string(fixedCount) +
               ", got " + std::to_string(args.size()) + " for " + expectedSignature(function);
    }

    for (size_t i = 0; i < fixedCount; ++i) {
        const auto& p = function.parameters[i];
        if (p.type == nullptr || args[i].type == nullptr || !p.type->assignableFrom(args[i])) {
            return "arguments are not compatible with function signature: argument #" + std::to_string(i + 1) +
                   " (" + p.name + ") is incompatible: expected " +
                   (p.type ? p.type->getName() : std::string("unknown")) + ", got " + valueTypeName(args[i]);
        }
    }

    if (variadic) {
        const auto& p = function.parameters.back();
        for (size_t i = fixedCount; i < args.size(); ++i) {
            if (p.type == nullptr || args[i].type == nullptr || !p.type->assignableFrom(args[i])) {
                return "arguments are not compatible with function signature: variadic argument #" + std::to_string(i + 1) +
                       " (" + p.name + ") is incompatible: expected " +
                       (p.type ? p.type->getName() : std::string("unknown")) + ", got " + valueTypeName(args[i]);
            }
        }
    }

    return "arguments are not compatible with function signature";
}

static Value makeCallError(const std::string& message, int line = -1, int col = -1) {
    return makeThrown("ArgumentException", message, line, col);
}

static bool bindCallArguments(
    const Function& function,
    const std::vector<Value>& args,
    std::vector<Value>& bound,
    std::string& error
) {
    if (!function.validArguments(args)) {
        error = describeArgumentMismatch(function, args);
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
        Value v = makeThrown("TypeException", "value is not a function");
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
        return makeCallError(bindError, callToken.line, callToken.pos + 1);
    }

    if (function.kind == FunctionKind::Internal) {
        if (!function.internalHandler) {
            return makeCallError("internal function is missing implementation");
        }
        runtimePushFrame(function.debugName, callToken.line, callToken.pos + 1);
        Value ret = function.internalHandler(scope, boundArgs, function.__this);
        runtimePopFrame();
        if (ret.thrownException != nullptr) {
            runtimeEnsureExceptionLocation(ret.thrownException, callToken.line, callToken.pos + 1);
        }
        return ret;
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
    runtimePushFrame(function.debugName, callToken.line, callToken.pos + 1);
    Value ret = function.body->execute(funcScope);
    runtimePopFrame();
    if (ret.thrownException != nullptr) {
        runtimeEnsureExceptionLocation(ret.thrownException, callToken.line, callToken.pos + 1);
    }
    return ret;
}
