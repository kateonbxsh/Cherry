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
        } else if (st->lastToken.line == line && st->lastToken.pos == c && furthestStatement != nullptr) {
            furthestStatement->expected.append_range(st->expected);
        }
    }

    return furthestStatement;
}

void Statement::invalidExpected(std::initializer_list<TokenKind> kinds, Lexer& lex) {
    valid = false;
    expected = tokenKindsToString(std::vector<TokenKind>(kinds));
    lastToken = lex.currentToken();
}

void Statement::invalidExpected(const std::vector<TokenKind>& kinds, Lexer& lex) {
    valid = false;
    expected = tokenKindsToString(kinds);
    lastToken = lex.currentToken();
}


void Statement::invalidFrom(const Statement& other) {
    valid = false;
    expected = other.expected;
    lastToken = other.lastToken;
}

Value NotAStatement::execute(Scope&)
{
    return NullValue;
}
