#include "statements/GlobalBlock.h"
#include "statements/definition/VariableDefinition.h"

GlobalBlock* GlobalBlock::parse(Lexer& lexer) {

    auto block = new GlobalBlock;

    while(lexer.peekToken().kind != EOF) {

        auto variableDefinition = VariableDefinition::parse(lexer);
        std::vector<Statement*> invalids = {};

        if (!variableDefinition->valid) {
            invalids.push_back(variableDefinition);
        } else {
            block->children.push_back(variableDefinition);
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