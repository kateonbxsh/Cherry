#include <data.h>
#include "statements/expression/Expression.h"
#include <expressions.h>

Expression *Expression::parse(Lexer &lexer) {

    std::cout << "parsing normal expression..." << std::endl;

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

    lexer.savePosition();
    auto potentialOperator = lexer.nextToken();
    if (isOperator(potentialOperator)) {
        expression->expressionOperator = potentialOperator;
    } else {
        lexer.rollPosition();
        lexer.deletePosition();
        deleteAllStatements(invalids);
        expression->valid = true;
        return expression;
    }

    //Found operator now we need the second operand
    auto potentialSecondOperand = Expression::parse(lexer);
    if (potentialSecondOperand->valid) {
        //complete expression
        expression->secondOperand = potentialSecondOperand;
        expression->valid = true;
        deleteAllStatements(invalids);
        lexer.deletePosition();

        //check precedence and inverse
        if (potentialSecondOperand->expressionOperator.kind != NONE) {
            int myPresedence = precedence(expression->expressionOperator);
            int childPresedence = precedence(potentialSecondOperand->expressionOperator);
            if (myPresedence > childPresedence) {
                //Rotate the tree
                auto childFirstOperand = potentialSecondOperand->firstOperand;
                expression->secondOperand = childFirstOperand;
                potentialSecondOperand->firstOperand = expression;
                return potentialSecondOperand;
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

     std::cout << "parsing paren expression..." << std::endl;

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

    std::cout << "parsing value expression..." << std::endl;

    lexer.savePosition();
    auto expression = new ExpressionValue;

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
    expression->expected = {IDENTIFIER, STRING, INTEGER, FLOAT};
    lexer.rollPosition();

    return expression;

}

Value Expression::execute(Scope& scope) {
    
    auto value1 = this->firstOperand->execute(scope);
    if (value1.thrownException != nullptr) return value1;

    if (this->expressionOperator.kind != NONE) {
        auto value2 = this->firstOperand->execute(scope);
        if (value2.thrownException != nullptr) return value2;

        return performOperator(value1, value2, this->expressionOperator.kind);

    }

    return value1;

}

Value ExpressionParenWrapped::execute(Scope &scope) {

    return this->child->execute(scope);

}

Value ExpressionValue::execute(Scope &scope) {

    auto token = this->identifier;

    switch(token.kind) {
        case INTEGER:
        {
            auto v = new int(std::stoi(token.value));
            return Value(IntegerType, v);
        }
        
        case FLOAT:
        {
            auto v = new float(std::stof(token.value));
            return Value(FloatType, v);
        }
            

        case NULL_TOKEN:
        {
            return NullValue;
        }

        case STRING:
        {
            auto v = new std::string(token.value);
            return Value(StringType, v);
        }
            

        case TRUE:
        case FALSE:
        {
            auto v = new bool(token.value == "true");
            return Value(BooleanType, v);
        }
            

        default: 
        {
            //TO-DO: return this when token is identifier and throw on default
            return scope.getVariable(token.value);
        }
    }

}
