#include "IfStatement.hpp"
#include <expressions.h>
#include "../GeneralStatement.h"

uref<IfStatement> IfStatement::parse(Lexer& lexer) {
    lexer.savePosition();

    auto ifStmt = create_unique<IfStatement>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering IfStatement::parse\n";

    Token next = lexer.nextToken();

    // --- Multi-clause block form ---
    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing block if/unless\n";

    while (true) {
        if (next.kind != IF && next.kind != UNLESS) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected IF or UNLESS\n";

            ifStmt->valid = false;
            ifStmt->expected = tokenKindsToString({IF, UNLESS});
            ifStmt->lastToken = next;
            lexer.rollPosition();
            return ifStmt;
        }

        IfClause clause;
        clause.isUnless = (next.kind == UNLESS);

        if (DEBUG) {
            std::cout << DEBUG_PREFIX
                      << "Parsing "
                      << (clause.isUnless ? "unless" : "if")
                      << " clause\n";
        }

        DEBUG_TABS++;
        clause.condition = Expression::parse(lexer);
        DEBUG_TABS--;

        if (!clause.condition->valid) {
            ifStmt->valid = false;
            ifStmt->expected = clause.condition->expected;
            ifStmt->lastToken = clause.condition->lastToken;
            lexer.rollPosition();
            return ifStmt;
        }

        auto parseClauseBody = [&]() -> uref<Block> {
            lexer.savePosition();
            auto block = Block::parse(lexer);
            if (block->valid) {
                lexer.deletePosition();
                return block;
            }
            lexer.rollPosition();

            lexer.savePosition();
            auto stmt = GeneralStatement::parse(lexer);
            if (!stmt->valid) {
                auto invalid = create_unique<Block>();
                invalid->valid = false;
                invalid->expected = stmt->expected;
                invalid->lastToken = stmt->lastToken;
                lexer.rollPosition();
                return invalid;
            }
            auto single = create_unique<Block>();
            single->valid = true;
            single->statements.push_back(move(stmt));
            lexer.deletePosition();
            return single;
        };

        if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing if/unless body\n";
        DEBUG_TABS++;
        clause.body = parseClauseBody();
        DEBUG_TABS--;

        if (!clause.body->valid) {
            ifStmt->valid = false;
            ifStmt->expected = clause.body->expected;
            ifStmt->lastToken = clause.body->lastToken;
            lexer.rollPosition();
            return ifStmt;
        }

        ifStmt->clauses.push_back(move(clause));

        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Clause parsed successfully\n";

        next = lexer.peekToken();
        if (next.kind == ELSE) {
            lexer.nextToken(); // consume ELSE
            Token lookahead = lexer.peekToken();

            if (lookahead.kind == IF || lookahead.kind == UNLESS) {
                if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing else-if / else-unless\n";

                lexer.nextToken(); // consume IF/UNLESS
                next = lookahead;
                continue;
            } else {
                if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing else block\n";

                auto elseBlock = parseClauseBody();
                if (!elseBlock->valid) {
                    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid else block\n";

                    ifStmt->valid = false;
                    ifStmt->expected = elseBlock->expected;
                    ifStmt->lastToken = elseBlock->lastToken;
                    lexer.rollPosition();
                    return ifStmt;
                }

                ifStmt->elseClause = move(elseBlock);

                if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Else block parsed\n";
                break;
            }
        } else {
            break;
        }
    }

    ifStmt->valid = true;

    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "IfStatement parsed successfully\n";
    lexer.deletePosition();
    return ifStmt;
}


Value IfStatement::execute(Scope& scope) {
    for (auto& clause : clauses) {
        Value condValue = clause.condition->execute(scope);
        if (condValue.thrownException != nullptr) return condValue;

        bool cond = isTruthy(condValue);
        if (clause.isUnless) cond = !cond;

        if (cond) return clause.body->execute(scope);
    }

    if (elseClause) return elseClause->execute(scope);

    return NullValue;
}
