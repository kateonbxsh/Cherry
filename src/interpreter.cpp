#include "interpreter.h"
#include "scope.h"
#include "expressions.h"

void Interpreter::interpret(uref<GlobalBlock>& block)
{

    Scope mainScope = Scope();

    auto returned = block->execute(mainScope);

    if (returned.thrownException != NULL) {
        std::cerr << "Exception: " << stringify(*returned.thrownException) << std::endl;
        return;
    }
    if (DEBUG) mainScope.printVariables();

}