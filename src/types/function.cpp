#include "function.h"
#include "class.h"
#include "data.h"

bool Function::validArguments(const std::vector<Value>& arguments) {
    
    // Arity check
    if (arguments.size() != parameters.size()) {
        return false;
    }

    // Type compatibility check
    for (size_t i = 0; i < arguments.size(); ++i) {
        const auto& argType = arguments[i];
        const auto& paramType = parameters[i].type;

        if (!paramType->assignableFrom(argType)) {
            return false;
        }
    }

    return true;
}
