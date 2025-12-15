#include "statements/conditional/IfStatement.hpp"

uref<IfStatement> IfStatement::parse(Lexer& lexer) {
    lexer.savePosition();

    auto ifStmt = create_unique<IfStatement>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering IfStatement::parse\n";

    Token next = lexer.nextToken();

    // --- One-line FunctionCall if/unless ---
    if (next.kind != IF && next.kind != UNLESS) {

        if (DEBUG) std::cout << DEBUG_PREFIX << "Trying one-line FunctionCall if/unless\n";

        lexer.back();
        DEBUG_TABS++;
        auto potentialCall = FunctionCall::parse(lexer);
        DEBUG_TABS--;

        if (potentialCall->valid) {
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "FunctionCall parsed successfully\n";

            next = lexer.nextToken();
            if (next.kind == IF || next.kind == UNLESS) {

                if (DEBUG) {
                    std::cout << DEBUG_PREFIX
                              << "Detected one-line "
                              << (next.kind == UNLESS ? "unless" : "if")
                              << "\n";
                }

                auto cond = Expression::parse(lexer);
                if (!cond->valid) {
                    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid condition in one-line if\n";

                    ifStmt->valid = false;
                    ifStmt->expected = cond->expected;
                    ifStmt->lastToken = cond->lastToken;
                    lexer.rollPosition();
                    return ifStmt;
                }

                if (!lexer.expectToken(SEMICOLON)) {
                    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Missing semicolon in one-line if\n";

                    ifStmt->valid = false;
                    ifStmt->expected = tokenKindsToString({SEMICOLON});
                    ifStmt->lastToken = lexer.nextToken();
                    lexer.rollPosition();
                    return ifStmt;
                }

                auto bodyBlock = create_unique<Block>();
                bodyBlock->statements.push_back(move(potentialCall));

                ifStmt->clauses.push_back({
                    move(cond),
                    move(bodyBlock),
                    next.kind == UNLESS
                });

                ifStmt->valid = true;

                if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed one-line if/unless statement\n";
                lexer.deletePosition();
                return ifStmt;
            } else {
                if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "FunctionCall not followed by if/unless\n";

                ifStmt->valid = false;
                ifStmt->expected = tokenKindsToString({IF, UNLESS});
                ifStmt->lastToken = lexer.nextToken();
                lexer.rollPosition();
                return ifStmt;
            }
        }

        if (DEBUG) std::cout << DEBUG_PREFIX << "Not a FunctionCall, trying block if/unless\n";

        lexer.rollPosition();
        next = lexer.nextToken();
    }

    // --- Multi-clause block form ---
    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing block if/unless\n";

    auto parseClause = [&](uref<Expression>& cond, uref<Block>& body) -> bool {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Parsing expression\n";
        DEBUG_TABS++;
        cond = Expression::parse(lexer);
        DEBUG_TABS--;
        if (!cond->valid) return false;

        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Parsing block\n";
        DEBUG_TABS++;
        body = Block::parse(lexer);
        DEBUG_TABS--;
        return body->valid;
    };

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

        if (!parseClause(clause.condition, clause.body)) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid condition or block in clause\n";

            ifStmt->valid = false;
            ifStmt->expected = {"condition or block"};
            ifStmt->lastToken = lexer.nextToken();
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

                auto elseBlock = Block::parse(lexer);
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
