#include "interpreter.h"
#include "scope.h"
#include "expressions.h"
#include "runtime_exception.h"

void Interpreter::interpret(uref<GlobalBlock>& block)
{

    Scope mainScope = Scope();

    auto returned = block->execute(mainScope);

    if (returned.thrownException != NULL) {
        printRuntimeException(returned.thrownException);
        return;
    }
    if (DEBUG) mainScope.printVariables();

}
