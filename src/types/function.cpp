#include "function.h"
#include "data.h"
#include <unordered_map>

bool Function::validArguments(const std::vector<Value>& arguments) const {
    auto sameType = [](const reference<Type>& a, const reference<Type>& b) -> bool {
        if (a == b) return true;
        if (a == nullptr || b == nullptr) return false;
        return !a->getName().empty() && a->getName() == b->getName();
    };

    auto acceptsWithBindings = [&](const reference<Type>& paramType, const Value& arg, const std::unordered_map<std::string, reference<Type>>& bindings) -> bool {
        if (paramType == nullptr || arg.type == nullptr) return false;

        if (paramType->kind == TypeKind::Dynamic) {
            const auto dynName = paramType->getName();
            if (!dynName.empty()) {
                auto it = bindings.find(dynName);
                if (it != bindings.end() && it->second != nullptr) {
                    return it->second->assignableFrom(arg);
                }
            }
        }

        return paramType->assignableFrom(arg);
    };

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

    std::unordered_map<std::string, reference<Type>> dependentBindings;

    for (size_t i = 0; i < fixedCount; ++i) {
        const auto& argType = arguments[i];
        const auto& paramType = parameters[i].type;
        if (!acceptsWithBindings(paramType, argType, dependentBindings)) {
            return false;
        }

        if (paramType == TypeType) {
            if (!std::holds_alternative<reference<Type>>(argType.value)) return false;
            auto chosen = std::get<reference<Type>>(argType.value);
            if (chosen == nullptr) return false;
            const auto& boundName = parameters[i].name;
            if (!boundName.empty()) {
                auto it = dependentBindings.find(boundName);
                if (it != dependentBindings.end()) {
                    if (!sameType(it->second, chosen)) return false;
                } else {
                    dependentBindings[boundName] = chosen;
                }
            }
        }
    }

    if (variadic) {
        const auto& variadicParamType = parameters.back().type;
        if (variadicParamType == nullptr) return false;
        for (size_t i = fixedCount; i < arguments.size(); ++i) {
            if (!acceptsWithBindings(variadicParamType, arguments[i], dependentBindings)) {
                return false;
            }
        }
    }

    return true;
}
