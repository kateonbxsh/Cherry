#include "Block.hpp"
#include "./expression/Expression.h"

uref<Block> Block::parse(Lexer& lexer) {

    lexer.savePosition();
    auto block = create_unique<Block>();

    if (lexer.expectToken(LEFT_BRACE)) {
        while (true) {
            while (lexer.expectToken(SEMICOLON)) {}
            if (lexer.expectToken(RIGHT_BRACE)) break;

            auto stmt = GeneralStatement::parse(lexer);

            if (!stmt->valid) {
                block->valid = false;
                block->expected = stmt->expected;
                block->lastToken = stmt->lastToken;
                lexer.rollPosition();
                return block;
            }

            block->statements.push_back(move(stmt));
        }

        block->valid = true;
        return block;
    }

    block->valid = false;
    block->expected = tokenKindsToString({LEFT_BRACE});
    block->lastToken = lexer.peekToken();
    lexer.rollPosition();
    return block;
}

Value Block::execute(Scope& scope) {
    Scope localScope = scope.createChild();
    Value result = NullValue;
    for (auto& stmt : statements) {
        result = stmt->execute(localScope);
        if (result.thrownException != nullptr) return result;
        if (result.returning) return result;
    }
    return result;
}
