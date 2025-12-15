#include <data.h>
#include "statements/expression/Expression.h"
#include "statements/function/FunctionCall.hpp"
#include <expressions.h>
#include <functional>

uref<Expression> Expression::parse(Lexer &lexer) {

    lexer.savePosition();

    auto expression = create_unique<Expression>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering Expression::parse\n";

    std::vector<uref<Statement>> invalids;
    bool foundFirstOperand = false;

    using ExprParser = std::function<uref<Expression>(Lexer&)>;
    std::vector<ExprParser> parsers = {
        [](Lexer& l) { return ExpressionParenWrapped::parse(l); },
        [](Lexer& l) { return FunctionCall::parse(l); },
        [](Lexer& l) { return ExpressionValue::parse(l); },
    };

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying to parse first operand\n";

    for (auto& parser : parsers) {
        DEBUG_TABS++;
        auto expr = parser(lexer);
        DEBUG_TABS--;
        if (expr->valid) {
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "First operand parsed successfully\n";
            expression->firstOperand = move(expr);
            foundFirstOperand = true;
            break;
        } else {
            if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "Operand parser failed\n";
            invalids.push_back(move(expr));
        }
    }

    if (!foundFirstOperand) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "No valid first operand found\n";

        lexer.rollPosition();
        auto furthest = getFurthestInvalidStatement(invalids);
        expression->expected = furthest->expected;
        expression->lastToken = furthest->lastToken;
        expression->valid = false;
        return expression;
    }

    expression->expressionOperator = {NONE, "", 0, 0};

    lexer.savePosition();
    auto potentialOperator = lexer.nextToken();

    if (isOperator(potentialOperator)) {
        if (DEBUG) {
            std::cout << DEBUG_PREFIX
                      << "Operator found: " << potentialOperator.value
                      << "\n";
        }
        expression->expressionOperator = potentialOperator;
    } else {
        if (DEBUG) std::cout << DEBUG_PREFIX << "No operator found, single operand expression\n";

        lexer.rollPosition();
        lexer.deletePosition();
        expression->valid = true;
        return expression;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing second operand\n";

    auto potentialSecondOperand = Expression::parse(lexer);
    if (potentialSecondOperand->valid) {

        expression->secondOperand = move(potentialSecondOperand);
        expression->valid = true;
        lexer.deletePosition();
        lexer.deletePosition();

        if (expression->secondOperand->expressionOperator.kind != NONE) {
            int myPrecedence = precedence(expression->expressionOperator);
            int childPrecedence = precedence(expression->secondOperand->expressionOperator);

            if (DEBUG) {
                std::cout << DEBUG_PREFIX
                          << "Checking precedence: parent=" << myPrecedence
                          << ", child=" << childPrecedence << "\n";
            }

            if (myPrecedence > childPrecedence) {
                if (DEBUG) std::cout << DEBUG_PREFIX << "Rotating expression tree\n";

                auto newRoot = move(expression->secondOperand);
                expression->secondOperand = move(newRoot->firstOperand);
                newRoot->firstOperand = move(expression);
                return newRoot;
            }
        }

        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Expression parsed successfully\n";
        return expression;
    }

    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid second operand\n";

    lexer.rollPosition();
    auto furthest = getFurthestInvalidStatement(invalids);
    expression->expected = furthest->expected;
    expression->lastToken = furthest->lastToken;
    expression->valid = false;
    return expression;
}


uref<Expression> ExpressionParenWrapped::parse(Lexer& lexer) {

    lexer.savePosition();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying ExpressionParenWrapped\n";

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "No '(' found\n";

        auto exp = create_unique<Expression>();
        exp->lastToken = lexer.nextToken();
        exp->expected = tokenKindsToString({LEFT_PARENTHESIS});
        exp->valid = false;
        lexer.rollPosition();
        return exp;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing inner expression\n";

    auto expression = Expression::parse(lexer);
    if (!expression->valid) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid expression inside parentheses\n";
        lexer.rollPosition();
        return expression;
    }

    if (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Missing ')'\n";

        expression->lastToken = lexer.nextToken();
        expression->expected = tokenKindsToString({RIGHT_PARENTHESIS});
        expression->valid = false;
        lexer.rollPosition();
        return expression;
    }

    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parenthesized expression parsed\n";

    lexer.deletePosition();
    return expression;
}

uref<ExpressionValue> ExpressionValue::parse(Lexer& lexer) {

    lexer.savePosition();
    auto expression = create_unique<ExpressionValue>();

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying ExpressionValue\n";

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
        if (DEBUG) {
            std::cout << DEBUG_SUCCESS_PREFIX
                      << "ExpressionValue parsed: "
                      << nextToken.value << "\n";
        }

        lexer.deletePosition();
        expression->identifier = nextToken;
        expression->valid = true;
        return expression;
    }

    if (DEBUG) {
        std::cout << DEBUG_WARNING_PREFIX
                  << "ExpressionValue failed, got "
                  << nextToken.value << "\n";
    }

    expression->valid = false;
    expression->lastToken = nextToken;
    expression->expected = tokenKindsToString({
        IDENTIFIER, STRING, INTEGER, FLOAT, TRUE, FALSE, NULL_TOKEN
    });
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
            boolean v = (token.kind == TRUE);
            return Value(v);
        }
            

        default: 
        {
            //TO-DO: return this when token is identifier and throw on default
            return scope.getVariable(token.value);
        }
    }

}
