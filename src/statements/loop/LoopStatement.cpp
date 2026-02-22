#include "LoopStatement.hpp"
#include "../Block.hpp"
#include "../variable/VariableAffectation.h"
#include "expressions.h"
#include "runtime_exception.h"

static inline bool shouldPropagateLoopValue(const Value& value) {
    return value.thrownException != nullptr || value.returning;
}

uref<Statement> ForStatement::parse(Lexer& lexer) {

    lexer.savePosition();
    auto stmt = create_unique<ForStatement>();

    if (!lexer.expectToken(FOR)) {
        stmt->invalidExpected({FOR}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        stmt->invalidExpected({LEFT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    auto init = GeneralStatement::parse(lexer);
    if (!init->valid) {
        stmt->invalidFrom(*init);
        lexer.rollPosition();
        return stmt;
    }

    auto cond = Expression::parse(lexer);
    if (!cond->valid) {
        stmt->invalidFrom(*cond);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(SEMICOLON)) {
        stmt->invalidExpected({SEMICOLON}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    uref<Statement> step;
    lexer.savePosition();
    auto stepAssign = VariableAffectation::parseWithoutSemicolon(lexer);
    if (stepAssign->valid) {
        step = move(stepAssign);
        lexer.deletePosition();
    } else {
        lexer.rollPosition();
        auto stepExpr = Expression::parse(lexer);
        if (!stepExpr->valid) {
            stmt->invalidFrom(*stepExpr);
            lexer.rollPosition();
            return stmt;
        }
        step = move(stepExpr);
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        stmt->invalidExpected({RIGHT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    // Optional semicolon for do-while form: do {...} while (cond);
    lexer.expectToken(SEMICOLON);

    auto body = Block::parse(lexer);
    if (!body->valid) {
        stmt->invalidFrom(*body);
        lexer.rollPosition();
        return stmt;
    }

    stmt->init = move(init);
    stmt->condition = move(cond);
    stmt->step = move(step);
    stmt->body = move(body);
    stmt->valid = true;

    lexer.deletePosition();
    return stmt;
}

Value ForStatement::execute(Scope& scope) {

    Value initVal = init->execute(scope);
    if (shouldPropagateLoopValue(initVal)) return initVal;

    while (true) {

        Value cond = condition->execute(scope);
        if (shouldPropagateLoopValue(cond)) return cond;
        if (!isTruthy(cond)) break;

        Value result = body->execute(scope);
        if (shouldPropagateLoopValue(result)) return result;

        Value stepVal = step->execute(scope);
        if (shouldPropagateLoopValue(stepVal)) return stepVal;
    }

    return NullValue;
}

uref<Statement> WhileStatement::parse(Lexer& lexer) {

    lexer.savePosition();
    auto stmt = create_unique<WhileStatement>();

    if (!lexer.expectToken(WHILE)) {
        stmt->valid = false;
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        stmt->invalidExpected({LEFT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    auto cond = Expression::parse(lexer);
    if (!cond->valid) {
        stmt->invalidFrom(*cond);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        stmt->invalidExpected({RIGHT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    // Optional semicolon for repeat-until form: repeat {...} until (cond);
    lexer.expectToken(SEMICOLON);

    auto body = Block::parse(lexer);
    if (!body->valid) {
        stmt->invalidFrom(*body);
        lexer.rollPosition();
        return stmt;
    }

    stmt->condition = move(cond);
    stmt->body = move(body);
    stmt->valid = true;

    lexer.deletePosition();
    return stmt;
}


Value WhileStatement::execute(Scope& scope) {

    while (true) {
        auto c = condition->execute(scope);
        if (shouldPropagateLoopValue(c)) return c;
        if (!isTruthy(c)) break;

        auto r = body->execute(scope);
        if (shouldPropagateLoopValue(r)) return r;
    }

    return NullValue;
}

uref<Statement> DoWhileStatement::parse(Lexer& lexer) {

    lexer.savePosition();
    auto stmt = create_unique<DoWhileStatement>();

    if (!lexer.expectToken(DO)) {
        stmt->invalidExpected({DO}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    auto body = Block::parse(lexer);
    if (!body->valid) {
        stmt->invalidFrom(*body);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(WHILE)) {
        stmt->invalidExpected({WHILE}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        stmt->invalidExpected({LEFT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    auto cond = Expression::parse(lexer);
    if (!cond->valid) {
        stmt->invalidFrom(*cond);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        stmt->invalidExpected({RIGHT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    stmt->body = move(body);
    stmt->condition = move(cond);
    stmt->valid = true;

    lexer.deletePosition();
    return stmt;
}

Value DoWhileStatement::execute(Scope& scope) {

    do {
        auto r = body->execute(scope);
        if (shouldPropagateLoopValue(r)) return r;

        auto c = condition->execute(scope);
        if (shouldPropagateLoopValue(c)) return c;

        if (!isTruthy(c)) break;

    } while (true);

    return NullValue;
}

uref<Statement> RepeatUntilStatement::parse(Lexer& lexer) {

    lexer.savePosition();
    auto stmt = create_unique<RepeatUntilStatement>();

    if (!lexer.expectToken(REPEAT)) {
        stmt->invalidExpected({REPEAT}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    auto body = Block::parse(lexer);
    if (!body->valid) {
        stmt->invalidFrom(*body);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(UNTIL)) {
        stmt->invalidExpected({UNTIL}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        stmt->invalidExpected({LEFT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    auto cond = Expression::parse(lexer);
    if (!cond->valid) {
        stmt->invalidFrom(*cond);
        lexer.rollPosition();
        return stmt;
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        stmt->invalidExpected({RIGHT_PARENTHESIS}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    stmt->body = move(body);
    stmt->condition = move(cond);
    stmt->valid = true;

    lexer.deletePosition();
    return stmt;
}

Value RepeatUntilStatement::execute(Scope& scope) {

    while (true) {

        auto r = body->execute(scope);
        if (shouldPropagateLoopValue(r)) return r;

        auto c = condition->execute(scope);
        if (shouldPropagateLoopValue(c)) return c;

        if (isTruthy(c)) break;
    }

    return NullValue;
}

uref<Statement> RepeatTimesStatement::parse(Lexer& lexer) {

    lexer.savePosition();
    auto stmt = create_unique<RepeatTimesStatement>();

    if (!lexer.expectToken(REPEAT)) {
        
        if (DEBUG) std::cout << DEBUG_PREFIX << "NO REPEAT FOUND, found: " << lexer.peekToken().kind << std::endl;
        stmt->invalidExpected({REPEAT}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing repeat n" << std::endl;

    auto count = Expression::parse(lexer);
    if (!count->valid) {
        stmt->invalidFrom(*count);
        lexer.rollPosition();
        return stmt;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing times n" << std::endl;

    if (!lexer.expectToken(REPEAT_TIMES)) {
        stmt->invalidExpected({REPEAT_TIMES}, lexer);
        lexer.rollPosition();
        return stmt;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing block" << std::endl;

    auto body = Block::parse(lexer);
    if (!body->valid) {
        stmt->invalidFrom(*body);
        lexer.rollPosition();
        return stmt;
    }

    stmt->count = move(count);
    stmt->body = move(body);
    stmt->valid = true;

    lexer.deletePosition();
    return stmt;
}

Value RepeatTimesStatement::execute(Scope& scope) {

    auto v = count->execute(scope);
    if (shouldPropagateLoopValue(v)) return v;

    if (v.type != IntegerType) {
        return makeThrown("TypeException", "repeat count must be integer");
    }

    int n = get<integer>(v.value);

    for (int i = 0; i < n; ++i) {
        auto r = body->execute(scope);
        if (shouldPropagateLoopValue(r)) return r;
    }

    return NullValue;
}

