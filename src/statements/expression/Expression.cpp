#include "Expression.h"
#include "expressions.h"

Expression *Expression::parse(Lexer &lexer) {

    lexer.savePosition();

    auto expression = new Expression();
    std::vector<Statement*> invalids = {};
    bool foundFirstOperand = false;

    auto parenWrapped = ExpressionParenWrapped::parse(lexer);
    if (parenWrapped->valid) {
        expression->firstOperand = parenWrapped;
        deleteAllStatements(invalids);
        foundFirstOperand = true;
    } else {
        invalids.push_back(parenWrapped);
    }

    if (!foundFirstOperand) {
        auto identifier = ExpressionValue::parse(lexer);
        if (identifier->valid) {
            expression->firstOperand = identifier;
            deleteAllStatements(invalids);
            foundFirstOperand = true;
        } else {
            invalids.push_back(identifier);
        }
    }

    //Cannot read any first operand, not a valid expression
    if (!foundFirstOperand) {
        lexer.rollPosition();
        auto furthest = getFurthestInvalidStatement(invalids);
        expression->expected = furthest->expected;
        expression->lastToken = furthest->lastToken;
        expression->valid = false;
        delete furthest;
        return expression;
    }

    //Found a valid first operand, we check if there is no second operand.
    expression->expressionOperator = {NONE};

    auto potentialOperator = lexer.nextToken();
    if (isOperator(potentialOperator)) {
        expression->expressionOperator = potentialOperator;
    } else {
        lexer.deletePosition();
        deleteAllStatements(invalids);
        expression->valid = true;
        return expression;
    }

    //Found operator now we need the second operand
    auto secondOperand = Expression::parse(lexer);
    if (secondOperand->valid) {
        //complete expression
        expression->secondOperand = secondOperand;
        expression->valid = true;
        deleteAllStatements(invalids);
        lexer.deletePosition();

        //check precedence and inverse
        if (secondOperand->expressionOperator.kind != NONE) {
            int myPresedence = precedence(expression->expressionOperator);
            int childPresedence = precedence(secondOperand->expressionOperator);
            if (myPresedence > childPresedence) {
                //Rotate the tree
                auto childFirstOperand = secondOperand->firstOperand;
                expression->secondOperand = childFirstOperand;
                secondOperand->firstOperand = expression;
                return secondOperand;
            }
        }

        return expression;
    }

    //Invalid second operand
    lexer.rollPosition();
    auto furthest = getFurthestInvalidStatement(invalids);
    expression->expected = furthest->expected;
    expression->lastToken = furthest->lastToken;
    expression->valid = false;
    delete furthest;
    return expression;

}

ExpressionParenWrapped* ExpressionParenWrapped::parse(Lexer& lexer) {

    lexer.savePosition();

    auto parenWrapped = new ExpressionParenWrapped;

    if (!lexer.expectToken(LEFT_BRACE)) {
        parenWrapped->lastToken = lexer.nextToken();
        parenWrapped->expected = {LEFT_BRACE};
        parenWrapped->valid = false;
        lexer.rollPosition();
        return parenWrapped;
    }

    auto expression = Expression::parse(lexer);
    if (!expression->valid) {
        parenWrapped->lastToken = expression->lastToken;
        parenWrapped->expected = expression->expected;
        parenWrapped->valid = false;
        lexer.rollPosition();
        delete expression;
        return parenWrapped;
    }

    if (!lexer.expectToken(RIGHT_BRACE)) {
        parenWrapped->lastToken = lexer.nextToken();
        parenWrapped->expected = {RIGHT_BRACE};
        parenWrapped->valid = false;
        lexer.rollPosition();
        delete expression;
        return parenWrapped;
    }

    parenWrapped->child = expression;
    parenWrapped->valid = true;
    lexer.deletePosition();
    return parenWrapped;

}

ExpressionValue* ExpressionValue::parse(Lexer& lexer) {

    lexer.savePosition();
    auto expression = new ExpressionValue;

    Token identifier = lexer.nextToken();
    if (
        identifier.kind == IDENTIFIER ||
        identifier.kind == PRIMITIVE_STRING ||
        identifier.kind == PRIMITIVE_INTEGER ||
        identifier.kind == PRIMITIVE_FLOAT) {
        
        lexer.deletePosition();
        expression->identifier = identifier;
        expression->valid = true;
        return expression;
    } 

    expression->valid = false;
    expression->lastToken = identifier;
    expression->expected = {TokenKind::IDENTIFIER, TokenKind::PRIMITIVE_STRING, TokenKind::PRIMITIVE_INTEGER, TokenKind::PRIMITIVE_FLOAT};

    return expression;

}

void Expression::execute() {
    Statement::execute();
}
