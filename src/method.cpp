#include "method.h"
#include "scope.h"
#include "executor.h"

Value Method::call(std::vector<Value> passedArgs, Scope& scope) {
    if (passedArgs.size() < arguments.size()) return {"null"};
    Scope newScope = scope;
    if (internal) return internalMethod(passedArgs);
    for(Method::Argument& arg : arguments) {
        newScope.setVariable(arg.name, passedArgs.front());
        passedArgs.erase(passedArgs.begin());
    }
    return Executor::executeInternal(block, newScope);
}
