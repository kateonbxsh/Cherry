#include "statement.h"

Statement* getFurthestInvalidStatement(std::vector<Statement*> statements) {

    int line = 0, c = 0;
    Statement* furthestStatement = nullptr;
    for(Statement* st : statements) {
        if (st->lastToken.line > line) {
            if (furthestStatement != nullptr) delete furthestStatement;
            furthestStatement = st;
        } else {
            if (st->lastToken.pos > c) {
                if (furthestStatement != nullptr) delete furthestStatement;
                furthestStatement = st;
            } else delete st;
        }
    }

    return furthestStatement;

}

void deleteAllStatements(std::vector<Statement*> statements) {
    for(Statement* st : statements) {
        delete st;
    }
}