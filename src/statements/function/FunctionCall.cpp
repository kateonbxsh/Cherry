#include "FunctionCall.hpp"
#include "types/type.h"
#include "types/function.h"
#include "FunctionDeclaration.hpp"

uref<FunctionCall> FunctionCall::parse(Lexer& lexer) {
    return create_unique<FunctionCall>();
}


Value FunctionCall::execute(Scope& scope) {

    auto func = Expression::execute(scope);

    if (func.type != FunctionType) {
        auto exception = create_reference<Value>(Value("value is not a function"));
        Value v;
        v.thrownException = exception;
        return v;
    }

    auto function = get<Function>(func.value);

    // Evaluate arguments
    std::vector<Value> argValues;
    for (auto& arg : arguments) {
        Value val = arg->execute(scope);
        if (val.thrownException != nullptr) return val;
        argValues.push_back(val);
    }

    // Execute function body in a new scope
    Scope funcScope(scope); // child scope
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        funcScope.setVariable(function.parameters[i].name, argValues[i]);
    }

    return function.body->execute(funcScope);
}
