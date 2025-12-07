#include <data.h>
#include "statements/expression/Expression.h"
#include <expressions.h>

uref<Expression> Expression::parse(Lexer &lexer) {

    lexer.savePosition();

    auto expression = create_unique<Expression>();
    std::vector<uref<Statement>> invalids = {};
    bool foundFirstOperand = false;

    auto parenWrapped = ExpressionParenWrapped::parse(lexer);
    if (parenWrapped->valid) {
        expression->firstOperand = move(parenWrapped);
        foundFirstOperand = true;
    } else {
        invalids.push_back(move(parenWrapped));
    }

    if (!foundFirstOperand) {
        auto identifier = ExpressionValue::parse(lexer);
        if (identifier->valid) {
            expression->firstOperand = move(identifier);
            foundFirstOperand = true;
        } else {
            invalids.push_back(move(identifier));
        }
    }

    //Cannot read any first operand, not a valid expression
    if (!foundFirstOperand) {
        lexer.rollPosition();
        auto furthest = getFurthestInvalidStatement(invalids);
        expression->expected = furthest->expected;
        expression->lastToken = furthest->lastToken;
        expression->valid = false;
        return expression;
    }

    //Found a valid first operand, we check if there is no second operand.
    expression->expressionOperator = {NONE, "", 0, 0};

    lexer.savePosition();
    auto potentialOperator = lexer.nextToken();
    if (isOperator(potentialOperator)) {
        expression->expressionOperator = potentialOperator;
    } else {
        lexer.rollPosition();
        lexer.deletePosition();
        expression->valid = true;
        return expression;
    }

    //Found operator now we need the second operand
    auto potentialSecondOperand = Expression::parse(lexer);
    if (potentialSecondOperand->valid) {

        //complete expression
        expression->secondOperand = move(potentialSecondOperand);
        expression->valid = true;
        lexer.deletePosition();

        //check precedence and inverse
        if (expression->secondOperand->expressionOperator.kind != NONE) {
            int myPresedence = precedence(expression->expressionOperator);
            int childPresedence = precedence(expression->secondOperand->expressionOperator);
            if (myPresedence > childPresedence) {
                //Rotate the tree
                auto newRoot = move(expression->secondOperand);
                expression->secondOperand = move(newRoot->firstOperand);
                newRoot->firstOperand = move(expression);
                return newRoot;
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
    return expression;

}

uref<Expression> ExpressionParenWrapped::parse(Lexer& lexer) {

    lexer.savePosition();

    if (!lexer.expectToken(LEFT_BRACE)) {
        auto exp = create_unique<Expression>();
        exp->lastToken = lexer.nextToken();
        exp->expected = tokenKindsToString({LEFT_BRACE});
        exp->valid = false;
        lexer.rollPosition();
        return exp;
    }

    auto expression = Expression::parse(lexer);
    if (!expression->valid) {
        lexer.rollPosition();
        return expression;
    }

    if (!lexer.expectToken(RIGHT_BRACE)) {
        expression->lastToken = lexer.nextToken();
        expression->expected = tokenKindsToString({RIGHT_BRACE});
        expression->valid = false;
        lexer.rollPosition();
        return expression;
    }

    lexer.deletePosition();
    return expression;

}

uref<ExpressionValue> ExpressionValue::parse(Lexer& lexer) {

    lexer.savePosition();
    auto expression = create_unique<ExpressionValue>();

    Token nextToken = lexer.nextToken();
    if (
        nextToken.kind == IDENTIFIER ||
        nextToken.kind == STRING ||
        nextToken.kind == INTEGER ||
        nextToken.kind == FLOAT ||
        nextToken.kind == TRUE ||
        nextToken.kind == FALSE ||
        nextToken.kind == NULL_TOKEN
    ) {
        
        lexer.deletePosition();
        expression->identifier = nextToken;
        expression->valid = true;
        return expression;
    } 

    expression->valid = false;
    expression->lastToken = nextToken;
    expression->expected = tokenKindsToString({IDENTIFIER, STRING, INTEGER, FLOAT, TRUE, FALSE, NULL_TOKEN});
    lexer.rollPosition();

    return expression;

}

Value Expression::execute(Scope& scope) {
    
    auto value1 = this->firstOperand->execute(scope);
    if (value1.thrownException != nullptr) return value1;

    if (this->expressionOperator.kind != NONE) {
        auto value2 = this->secondOperand->execute(scope);
        if (value2.thrownException != nullptr) return value2;

        return performOperator(value1, value2, this->expressionOperator.kind);

    }

    return value1;

}

Value ExpressionValue::execute(Scope &scope) {

    auto token = this->identifier;

    switch(token.kind) {
        case INTEGER:
        {
            return Value((integer) std::stol(token.value));
        }
        
        case FLOAT:
        {
            return Value((real) std::stod(token.value));
        }
            

        case NULL_TOKEN:
        {
            return NullValue;
        }

        case STRING:
        {;
            return Value(token.value);
        }
            

        case TRUE:
        case FALSE:
        {
            boolean v = (token.value == "true");
            return Value(v);
        }
            

        default: 
        {
            //TO-DO: return this when token is identifier and throw on default
            return scope.getVariable(token.value);
        }
    }

}
