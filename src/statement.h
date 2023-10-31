#pragma once

#include <vector>
#include <string>

enum StatementType {

    NOT_A_STATEMENT,
    VARIABLE_DEFINITION,
    VARIABLE_AFFECTATION,
    IF_STATEMENT,
    RETURN_STATEMENT

};

class Statement {

public:
    Statement() = default;

    int pos = 0; int line = 0;

    StatementType getType() const {
        return statementType;
    }

    virtual void execute() {};

protected:
    explicit Statement(StatementType type) : statementType(type) {}

private:
    StatementType statementType = NOT_A_STATEMENT;
};

struct Block {

    std::vector<Statement*> statements{};

};