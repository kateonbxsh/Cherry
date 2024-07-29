#include "statement.h"

Statement* getFurthestInvalidStatement(const std::vector<Statement*>& statements) {

    int line = 0, c = 0;
    Statement* furthestStatement = nullptr;
    for(Statement* st : statements) {
        if (st->lastToken.line > line) {
            delete furthestStatement;
            furthestStatement = st;
            line = st->lastToken.line, c = st->lastToken.pos;
        } else if (st->lastToken.line == line && st->lastToken.pos > c) {
            delete furthestStatement;
            furthestStatement = st;
            line = st->lastToken.line, c = st->lastToken.pos;
        } else delete st;
    }

    return furthestStatement;

}

void deleteAllStatements(std::vector<Statement*>& statements) {
    for(Statement* st : statements) {
        delete st;
    }
    statements.clear();
}