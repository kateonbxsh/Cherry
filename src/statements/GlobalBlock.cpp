#include "statements/GlobalBlock.h"
#include "statements/variable/VariableDefinition.h"

GlobalBlock* GlobalBlock::parse(Lexer& lexer) {

    auto block = new GlobalBlock;

    while(!lexer.expectToken(END_OF_FEED)) {

        auto variableDefinition = VariableDefinition::parse(lexer);
        std::vector<Statement*> invalids = {};

        if (!variableDefinition->valid) {
            invalids.push_back(variableDefinition);
        } else {
            block->children.push_back(variableDefinition);
            deleteAllStatements(invalids);
            continue;
        }
        
        //no valid statement
        auto furthest = getFurthestInvalidStatement(invalids);
        deleteAllStatements(block->children);
        block->valid = false;
        block->expected = furthest->expected;
        block->lastToken = furthest->lastToken;
        delete furthest;
        return block;

    }

    block->valid = true;
    return block;

}

Value GlobalBlock::execute(Scope& scope) {

    Value lastValue;

    for (auto childStatement : this->children) {
        lastValue = childStatement->execute(scope);
        if (lastValue.thrownException != nullptr) return lastValue;
    }
    
    return lastValue;

}