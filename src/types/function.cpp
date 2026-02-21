#include "function.h"
#include "data.h"

bool Function::validArguments(const std::vector<Value>& arguments) const {
    size_t fixedCount = parameters.size();
    bool variadic = false;
    if (!parameters.empty() && parameters.back().variadic) {
        variadic = true;
        fixedCount--;
    }

    if (variadic) {
        if (arguments.size() < fixedCount) return false;
    } else if (arguments.size() != fixedCount) {
        return false;
    }

    for (size_t i = 0; i < fixedCount; ++i) {
        const auto& argType = arguments[i];
        const auto& paramType = parameters[i].type;
        if (paramType == nullptr || argType.type == nullptr) return false;
        if (!paramType->assignableFrom(argType)) {
            return false;
        }
    }

    if (variadic) {
        const auto& variadicParamType = parameters.back().type;
        if (variadicParamType == nullptr) return false;
        for (size_t i = fixedCount; i < arguments.size(); ++i) {
            if (arguments[i].type == nullptr) return false;
            if (!variadicParamType->assignableFrom(arguments[i])) {
                return false;
            }
        }
    }

    return true;
}
