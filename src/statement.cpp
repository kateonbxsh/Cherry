#include "statement.h"

uref<Statement> getFurthestInvalidStatement(const std::vector<uref<Statement>>& statements) {
    
    int line = 0, c = 0;
    std::unique_ptr<Statement> furthestStatement = nullptr;

    for (const auto& st : statements) {
        if (st->lastToken.line > line || (st->lastToken.line == line && st->lastToken.pos > c)) {
            furthestStatement = create_unique<NotAStatement>();
            line = st->lastToken.line;
            c = st->lastToken.pos;
            furthestStatement->lastToken = st->lastToken;
            furthestStatement->expected = st->expected;
            furthestStatement->valid = false;
        }
    }

    return furthestStatement;
}

Value NotAStatement::execute(Scope &scope)
{
    return NullValue;
}
