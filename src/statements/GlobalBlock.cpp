#include "statements/GlobalBlock.h"
#include "statements/variable/VariableDefinition.h"
#include "statements/GeneralStatement.h"
#include <iostream>
#include "macros.h"

uref<GlobalBlock> GlobalBlock::parse(Lexer& lexer) {

    auto block = create_unique<GlobalBlock>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering GlobalBlock::parse\n";

    while(!lexer.expectToken(END_OF_FEED)) {

        if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing next general statement...\n";

        auto nextStatement = GeneralStatement::parse(lexer);

        if (DEBUG && nextStatement) {
            std::cout << DEBUG_SUCCESS_PREFIX << "Parsed statement, valid: " << nextStatement->valid << "\n";
        }

        if (!nextStatement->valid) {
            if (DEBUG) {
                std::cout << DEBUG_ERROR_PREFIX << "Invalid statement encountered." << "\n";
            }
            block->expected = nextStatement->expected;
            block->lastToken = nextStatement->lastToken;
            block->valid = false;
            if (DEBUG) std::cout << DEBUG_PREFIX << "Exiting GlobalBlock::parse with invalid block.\n";
            return block;
        }

        block->children.push_back(move(nextStatement));

        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Statement added to children. Children count: " << block->children.size() << "\n";
    }

    block->valid = true;
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Exiting GlobalBlock::parse with valid block.\n";
    return block;

}

Value GlobalBlock::execute(Scope& scope) {

    Value lastValue;

    if (DEBUG) {
        std::cout << DEBUG_PREFIX << "Entering GlobalBlock::execute\n";
        std::cout << DEBUG_PREFIX << "Children count: " << this->children.size() << "\n";
    }

    for (auto& childStatement : this->children) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Executing child statement...\n";
        lastValue = childStatement->execute(scope);
        if (DEBUG) {
            std::cout << DEBUG_PREFIX << "Child statement executed. ThrownException: "
                      << (lastValue.thrownException != nullptr ? "YES" : "NO") << "\n";
        }
        if (lastValue.thrownException != nullptr) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Exception thrown. Exiting GlobalBlock::execute early.\n";
            return lastValue;
        }
    }
    
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Exiting GlobalBlock::execute\n";
    return lastValue;
}