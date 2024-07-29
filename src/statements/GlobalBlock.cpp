#include "statements/GlobalBlock.h"
#include "statements/definition/VariableDefinition.h"

GlobalBlock* GlobalBlock::parse(Lexer& lexer) {

    std::cout << "parsing global block" << std::endl;

    auto block = new GlobalBlock;

    while(!lexer.expectToken(END_OF_FEED)) {

        std::cout << "looking for valid statement" << std::endl;

        auto variableDefinition = VariableDefinition::parse(lexer);
        std::vector<Statement*> invalids = {};

        if (!variableDefinition->valid) {
            invalids.push_back(variableDefinition);
        } else {
            std::cout << "found valid, continuing" << std::endl;
            block->children.push_back(variableDefinition);
            deleteAllStatements(invalids);
            continue;
        }

        std::cout << "all invalid, no valid statement" << std::endl;

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