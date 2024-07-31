#include "statement.h"

unique<Statement> getFurthestInvalidStatement(const std::vector<unique<Statement>>& statements) {
    
    int line = 0, c = 0;
    std::unique_ptr<Statement> furthestStatement = nullptr;

    for (const auto& st : statements) {
        if (st->lastToken.line > line || (st->lastToken.line == line && st->lastToken.pos > c)) {
            furthestStatement = std::make_unique<Statement>();
            line = st->lastToken.line;
            c = st->lastToken.pos;
            furthestStatement->lastToken = st->lastToken;
            furthestStatement->expected = st->expected;
            furthestStatement->valid = false;
        }
    }

    return furthestStatement;
}