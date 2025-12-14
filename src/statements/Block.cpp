#include "Block.hpp"

uref<Block> Block::parse(Lexer& lexer) {

    lexer.savePosition();
    auto block = create_unique<Block>();

    Token next = lexer.nextToken();

    // Case 1: Curly-brace block { ... }
    if (next.kind == LEFT_BRACE) {
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

    // Case 2: Inline single statement
    lexer.back(); // put back the token for statement parsing
    auto stmt = GeneralStatement::parse(lexer);

    if (!stmt->valid) {
        block->valid = false;
        block->expected = stmt->expected;
        block->lastToken = stmt->lastToken;
        lexer.rollPosition();
        return block;
    }

    block->statements.push_back(move(stmt));
    block->valid = true;
    return block;
}

Value Block::execute(Scope& scope) {
    Value result = NullValue;
    for (auto& stmt : statements) {
        result = stmt->execute(scope);
        if (result.thrownException != nullptr) return result;
    }
    return result;
}
