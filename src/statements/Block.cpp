#include "Block.hpp"
#include "./expression/Expression.h"

uref<Block> Block::parse(Lexer& lexer) {

    lexer.savePosition();
    auto block = create_unique<Block>();

    if (lexer.expectToken(LEFT_BRACE)) {
        while (!lexer.expectToken(RIGHT_BRACE)) {

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

    auto stmt = Expression::parse(lexer);

    if (!stmt->valid) {
        block->valid = false;
        block->expected = stmt->expected;
        block->lastToken = stmt->lastToken;
        lexer.rollPosition();
        return block;
    }

    block->statements.push_back(move(stmt));
    block->valid = true;
    lexer.deletePosition();
    return block;
}

Value Block::execute(Scope& scope) {
    Value result = NullValue;
    for (auto& stmt : statements) {
        result = stmt->execute(scope);
        if (result.thrownException != nullptr) return result;
        if (result.returning) return result;
    }
    return result;
}
