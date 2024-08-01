#include "statements/GlobalBlock.h"
#include "statements/variable/VariableDefinition.h"
#include "statements/GeneralStatement.h"

uref<GlobalBlock> GlobalBlock::parse(Lexer& lexer) {

    auto block = create_unique<GlobalBlock>();

    while(!lexer.expectToken(END_OF_FEED)) {

        auto nextStatement = GeneralStatement::parse(lexer);

        if (!nextStatement->valid) {
            block->expected = nextStatement->expected;
            block->lastToken = nextStatement->lastToken;
            block->valid = false;
            return block;
        }

        block->children.push_back(move(nextStatement));

    }

    block->valid = true;
    return block;

}

Value GlobalBlock::execute(Scope& scope) {

    Value lastValue;

    for (auto& childStatement : this->children) {
        lastValue = childStatement->execute(scope);
        if (lastValue.thrownException != nullptr) return lastValue;
    }
    
    return lastValue;

}