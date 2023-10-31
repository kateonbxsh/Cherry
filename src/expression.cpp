#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <utility>
#include "compiler.h"
#include "executor.h"

bool isTruthy(const Value& value) {
    if (value.type == "int" && std::any_cast<int>(value.value) > 0) return true;
    if (value.type == "string" && std::any_cast<std::string>(value.value).length() > 0) return true;
    if (value.type == "float" && std::any_cast<float>(value.value) > 0.5f) return true;
    if (value.type == "bool") return std::any_cast<bool>(value.value);
    return false;
}
bool isNumeric(const Value& value) {
    return (value.type == "int" || value.type == "bool" || value.type == "float" || value.type == "null");
}
std::string stringify(const Value& value) {
    if (value.type == "str") return std::any_cast<std::string>(value.value);
    if (value.type == "int") return std::to_string(std::any_cast<int>(value.value));
    if (value.type == "float") return std::to_string(std::any_cast<float>(value.value));
    if (value.type == "bool") return std::to_string(std::any_cast<bool>(value.value));
    if (value.type == "null") return "null";
    return "<object>";
}
float numericToFloat(const Value& value) {
    if (value.type == "int") return static_cast<float>(std::any_cast<int>(value.value));
    if (value.type == "float") return std::any_cast<float>(value.value);
    if (value.type == "bool") return static_cast<float>(std::any_cast<bool>(value.value));
    return 0.0f;
}
int numericToInt(const Value& value) {
    if (value.type == "float") return static_cast<int>(std::any_cast<int>(value.value));
    if (value.type == "int") return std::any_cast<int>(value.value);
    if (value.type == "bool") return static_cast<int>(std::any_cast<bool>(value.value));
    return 0;
}

bool isOperator(const Token& token) {
    TokenKind kind = token.kind;
    return (kind == PLUS || kind == MINUS || kind == TIMES || kind == DIVIDE);
}

int precedence(const Token& token) {
    TokenKind kind = token.kind;
    if (kind == TIMES || kind == DIVIDE) {
        return 2;
    } else if (kind == PLUS || kind == MINUS) {
        return 1;
    }
    return 0;
}

Value Executor::evaluateExpression(TokenList tokenList, Scope& scope) {
    std::stack<Value> valueStack;

    Token token;

    int i = 0;
    while (i < tokenList.size()) {
        token = tokenList[i];
        if (token.kind == NAME) {
            valueStack.push(scope.getVariable(token.value));
        } else if (token.kind == STRING) {
            valueStack.push({"str", token.value, false});
        } else if (token.kind == FLOAT) {
            float value = std::stof(token.value);
            valueStack.push({"float", value, false});
        } else if (token.kind == INTEGER) {
            int value = std::stoi(token.value);
            valueStack.push({"int", value, false});
        } else {
            if (valueStack.size() < 2) {
                std::cerr << "Invalid expression" << std::endl;
                return {"null", 0, false};
            }

            Value rightOperand = valueStack.top();
            valueStack.pop();
            Value leftOperand = valueStack.top();
            valueStack.pop();

            Value result;
            switch (token.kind) {
                case PLUS:
                    if (leftOperand.type == "str" && rightOperand.type == "str") {
                        std::string res = stringify(leftOperand)
                                + stringify(rightOperand);
                        result = {"str", res, false};
                    } else if ((leftOperand.type == "str" || rightOperand.type == "str")) {
                        std::string res = stringify(leftOperand) + stringify(rightOperand);
                        result = {"str", res, false};
                    } else if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        if (leftOperand.type == "float" || rightOperand.type == "float") {
                            result = {"float",
                                      numericToFloat(leftOperand) + numericToFloat(rightOperand)
                                      , false};
                            break;
                        }
                        result = {"int",
                                  numericToInt(leftOperand) + numericToInt(rightOperand),
                                  false};
                    }
                    break;
                case MINUS:
                    if (leftOperand.type == "str" && rightOperand.type == "str") {
                        std::string left = stringify(leftOperand), right = stringify(rightOperand);
                        size_t pos = left.find(right);
                        while(pos != std::string::npos) {
                            left.replace(pos, right.length(), "");
                            pos = left.find(right);
                        }
                        result = {"str", left, false};
                    } else if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        if (leftOperand.type == "float" || rightOperand.type == "float") {
                            result = {"float",
                                      numericToFloat(leftOperand) - numericToFloat(rightOperand)
                                    , false};
                            break;
                        }
                        result = {"int",
                                  numericToInt(leftOperand) - numericToInt(rightOperand),
                                  false};
                    }
                    break;
                case TIMES:
                    if ((leftOperand.type == "str" || rightOperand.type == "str")
                    &&  (isNumeric(leftOperand) || (isNumeric(rightOperand)))) {
                        int times; std::string str;
                        if (leftOperand.type == "str") { str = stringify(leftOperand); times = numericToInt(rightOperand); }
                        else { str = stringify(rightOperand); times = numericToInt(leftOperand); }
                        std::string res;
                        while (times > 0) {
                            res += str; times--;
                        }
                        result = {"str", res, false};
                    } else if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        if (leftOperand.type == "float" || rightOperand.type == "float") {
                            result = {"float",
                                      numericToFloat(leftOperand) * numericToFloat(rightOperand)
                                    , false};
                            break;
                        }
                        result = {"int",
                                  numericToInt(leftOperand) * numericToInt(rightOperand),
                                  false};
                    }
                    break;
                case DIVIDE:
                    if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        float deno = numericToFloat(rightOperand);
                        if (deno != 0) {
                            result = {"float",
                                      numericToFloat(leftOperand) / deno, false};
                        }
                    }
                    break;
                default:
                    std::cerr << "Invalid operator: " << token.value << std::endl;
                    return {"null", 0};
            }
            valueStack.push(result);
        }
        i++;
    }

    if (valueStack.size() == 1) {
        return valueStack.top();
    } else {
        std::cerr << "Invalid expression" << std::endl;
        return {"null", 0};
    }
}

// Function to convert an infix expression to Reverse Polish Notation (RPN)
TokenList Compiler::parseExpression() {
    std::vector<Token> output;
    std::stack<Token> operatorStack;

    while (lexer.hasNextToken()) {
        Token token = lexer.nextToken();
        Token nextToken;
        Token incomingToken;

        if (token.kind == NAME) {
            output.push_back(token);
        } else if (token.kind == INTEGER || token.kind == FLOAT || token.kind == STRING) {
            output.push_back(token);
        } else if (isOperator(token)) {
            while (!operatorStack.empty() && precedence(token) <= precedence(operatorStack.top())) {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
            operatorStack.push(token);
        } else if (token.kind == TokenKind::LEFT_BRACE) {
            operatorStack.push(token);
        } else if (token.kind == TokenKind::RIGHT_BRACE) {
            while (!operatorStack.empty() && operatorStack.top().kind != TokenKind::LEFT_BRACE) {
                output.push_back(operatorStack.top());
                operatorStack.pop();
            }
            if (!operatorStack.empty() && operatorStack.top().kind == TokenKind::LEFT_BRACE) {
                operatorStack.pop();
            } else {
                lexer.moveReader(-1);
                break;
            }
        } else {
            lexer.moveReader(-1);
            break;
        }
    }

    while (!operatorStack.empty()) {
        if (operatorStack.top().kind == LEFT_BRACE || operatorStack.top().kind == RIGHT_BRACE) {
            std::cerr << "Mismatched parentheses" << std::endl;
            return {};
        }
        output.push_back(operatorStack.top());
        operatorStack.pop();
    }

    return output;
}