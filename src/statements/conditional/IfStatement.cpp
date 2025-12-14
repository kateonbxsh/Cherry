#include "statements/conditional/IfStatement.hpp"

uref<IfStatement> IfStatement::parse(Lexer& lexer) {
    lexer.savePosition();

    auto ifStmt = create_unique<IfStatement>();
    Token next = lexer.nextToken();

    // --- One-line FunctionCall if/unless ---
    if (next.kind != IF && next.kind != UNLESS) {
        lexer.back();
        auto potentialCall = FunctionCall::parse(lexer);
        if (potentialCall->valid) {
            next = lexer.nextToken();
            if (next.kind == IF || next.kind == UNLESS) {
                auto cond = Expression::parse(lexer);
                if (!cond->valid) {
                    ifStmt->valid = false;
                    ifStmt->expected = cond->expected;
                    ifStmt->lastToken = cond->lastToken;
                    lexer.rollPosition();
                    return ifStmt;
                }

                if (!lexer.expectToken(SEMICOLON)) {
                    ifStmt->valid = false;
                    ifStmt->expected = tokenKindsToString({SEMICOLON});
                    ifStmt->lastToken = lexer.nextToken();
                    lexer.rollPosition();
                    return ifStmt;
                }

                auto bodyBlock = create_unique<Block>();
                bodyBlock->statements.push_back(move(potentialCall));

                ifStmt->clauses.push_back({move(cond), move(bodyBlock), next.kind == UNLESS});
                ifStmt->valid = true;
                return ifStmt;
            } else {
                // Not an if/unless after FunctionCall → not an IfStatement
                lexer.rollPosition();
                ifStmt->valid = false;
                ifStmt->expected = tokenKindsToString({IF, UNLESS});
                ifStmt->lastToken = next;
                return ifStmt;
            }
        }
        // Not a FunctionCall → roll back and try normal if/unless
        lexer.rollPosition();
        next = lexer.nextToken();
    }

    // --- Multi-clause block form ---
    auto parseClause = [&](uref<Expression>& cond, uref<Block>& body, bool& isUnless) -> bool {
        cond = Expression::parse(lexer);
        if (!cond->valid) return false;
        body = Block::parse(lexer);
        return body->valid;
    };

    while (true) {
        if (next.kind != IF && next.kind != UNLESS) {
            ifStmt->valid = false;
            ifStmt->expected = tokenKindsToString({IF, UNLESS});
            ifStmt->lastToken = next;
            lexer.rollPosition();
            return ifStmt;
        }

        IfClause clause;
        clause.isUnless = (next.kind == UNLESS);

        if (!parseClause(clause.condition, clause.body, clause.isUnless)) {
            ifStmt->valid = false;
            ifStmt->expected = {"condition or block"};
            ifStmt->lastToken = lexer.nextToken();
            lexer.rollPosition();
            return ifStmt;
        }

        ifStmt->clauses.push_back(move(clause));

        next = lexer.peekToken();
        if (next.kind == ELSE) {
            lexer.nextToken(); // consume ELSE
            Token lookahead = lexer.peekToken();
            if (lookahead.kind == IF || lookahead.kind == UNLESS) {
                lexer.nextToken(); // consume ELSE IF/UNLESS
                next = lookahead;
                continue; // parse next clause
            } else {
                // ELSE block
                auto elseBlock = Block::parse(lexer);
                if (!elseBlock->valid) {
                    ifStmt->valid = false;
                    ifStmt->expected = elseBlock->expected;
                    ifStmt->lastToken = elseBlock->lastToken;
                    lexer.rollPosition();
                    return ifStmt;
                }
                ifStmt->elseClause = move(elseBlock);
                break;
            }
        } else {
            break;
        }
    }

    ifStmt->valid = true;
    return ifStmt;
}

Value IfStatement::execute(Scope& scope) {
    /*for (auto& clause : clauses) {
        Value condValue = clause.condition->execute(scope);
        if (condValue.thrownException != nullptr) return condValue;

        bool cond = condValue.asBool();
        if (clause.isUnless) cond = !cond;

        if (cond) return clause.body->execute(scope);
    }

    if (elseClause) return elseClause->execute(scope);

    return NullValue;*/
    return NullValue;
}
