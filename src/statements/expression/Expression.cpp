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
        auto identifier = ExpressionIdentifier::parse(lexer);
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

    auto paren = lexer.nextToken();
    if (paren.kind != LEFT_BRACE) {
        parenWrapped->lastToken = paren;
        parenWrapped->expected = {LEFT_BRACE};
        parenWrapped->valid = false;
        return parenWrapped;
    }

    auto expression = Expression::parse(lexer);
    if (!expression->valid) {
        parenWrapped->lastToken = expression->lastToken;
        parenWrapped->expected = expression->expected;
        parenWrapped->valid = false;
        delete expression;
        return parenWrapped;
    }

    paren = lexer.nextToken();
    if (paren.kind != RIGHT_BRACE) {
        parenWrapped->lastToken = paren;
        parenWrapped->expected = {RIGHT_BRACE};
        parenWrapped->valid = false;
        delete expression;
        return parenWrapped;
    }

    parenWrapped->child = expression;
    parenWrapped->valid = true;
    return parenWrapped;

}

void Expression::execute() {
    Statement::execute();
}
