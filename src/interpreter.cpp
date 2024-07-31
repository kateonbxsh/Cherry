#include "interpreter.h"
#include "scope.h"

void Interpreter::interpret(unique<GlobalBlock>& block)
{

    Scope mainScope = Scope();

    auto returned = block->execute(mainScope);

    if (returned.thrownException) {
        std::cerr << "ExceptionThrown!" << std::endl;
        return;
    }

    mainScope.printVariables();

}