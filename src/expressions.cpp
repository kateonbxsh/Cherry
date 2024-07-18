#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <utility>
#include <valarray>
#include "compiler.h"
#include "executor.h"

bool firstIsBigger(const Value& value1, const Value& value2) {
    if (value1.type != value2.type) return false;
    if (value1.type == "float") return (std::any_cast<float>(value1.value) > std::any_cast<float>(value2.value));
    if (value1.type == "string") return (std::any_cast<std::string>(value1.value) > std::any_cast<std::string>(value2.value));
    if (value1.type == "int") return (std::any_cast<int>(value1.value) > std::any_cast<int>(value2.value));
    if (value1.type == "bool") return (std::any_cast<bool>(value1.value) > std::any_cast<bool>(value2.value));
    return false;
}

bool compareValues(const Value& value1, const Value& value2) {
    if (value1.type != value2.type) return false;
    if (value1.type == "float") return (std::any_cast<float>(value1.value) == std::any_cast<float>(value2.value));
    if (value1.type == "string") return (std::any_cast<std::string>(value1.value) == std::any_cast<std::string>(value2.value));
    if (value1.type == "int") return (std::any_cast<int>(value1.value) == std::any_cast<int>(value2.value));
    if (value1.type == "bool") return (std::any_cast<bool>(value1.value) == std::any_cast<bool>(value2.value));
    if (value1.type == "null") return true;
    return false;
}

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
    if (value.type == "string") return std::any_cast<std::string>(value.value);
    if (value.type == "int") return std::to_string(std::any_cast<int>(value.value));
    if (value.type == "float") return std::to_string(std::any_cast<float>(value.value));
    if (value.type == "bool") return std::to_string(std::any_cast<bool>(value.value));
    if (value.type == "null") return "null";
    return "<object " + value.type + ">";
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
    return (kind > BEGIN_OF_OPERATORS && kind < END_OF_OPERATORS);
}

int precedence(const Token& token) {
    TokenKind kind = token.kind;
    if (kind == NOT || kind == AND || kind == OR) {
        return 3;
    } else if (kind == TIMES || kind == DIVIDE) {
        return 2;
    } else if (kind == PLUS || kind == MINUS) {
        return 1;
    }
    return kind != LEFT_BRACE;
}

Value Executor::evaluateExpression(TokenList tokenList, Scope& scope) {
    std::stack<Value> valueStack;

    Token token;

    int i = 0;
    while (i < tokenList.size()) {
        token = tokenList[i];
        if (token.kind == IDENTIFIER && scope.hasVariable(token.value)) {
            valueStack.push(scope.getVariable(token.value));
        } else if (token.kind == STRING) {
            valueStack.push({"string", token.value, false});
        } else if (token.kind == FLOAT) {
            float value = std::stof(token.value);
            valueStack.push({"float", value, false});
        } else if (token.kind == INTEGER) {
            int value = std::stoi(token.value);
            valueStack.push({"int", value, false});
        } else {

            Value rightOperand;
            Value leftOperand;

            if (token.kind == IDENTIFIER && !scope.hasMethod(token.value)) continue;

            Value result = {"null", 0};
            switch (token.kind) {
                case IDENTIFIER: {
                    Method method = scope.getMethod(token.value);
                    if (valueStack.empty()) break;
                    int passedArgumentCount = numericToInt(valueStack.top());
                    if (valueStack.size() < passedArgumentCount) break;
                    valueStack.pop();
                    std::vector<Value> passedArguments = {};
                    while (passedArgumentCount-- > 0) {
                        passedArguments.insert(passedArguments.begin(), valueStack.top());
                        valueStack.pop();
                    }
                    result = method.call(passedArguments, scope);
                    break;
                }
                case BITWISE_XOR: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        int a = numericToInt(leftOperand), b = numericToInt(rightOperand);
                        result = {"int", a ^ b, false};
                    }
                    break;
                }
                case BITWISE_OR: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        int a = numericToInt(leftOperand), b = numericToInt(rightOperand);
                        result = {"int", a | b, false};
                    }
                    break;
                }
                case BITWISE_AND: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (isNumeric(leftOperand) && isNumeric(rightOperand)) {
                        int a = numericToInt(leftOperand), b = numericToInt(rightOperand);
                        result = {"int", a & b, false};
                    }
                    break;
                }
                case XOR: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    result = {"bool", isTruthy(leftOperand) ^ isTruthy(rightOperand), false};
                    break;
                }
                case OR: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    result = {"bool", isTruthy(leftOperand) || isTruthy(rightOperand), false};
                    break;
                }
                case AND: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    result = {"bool", isTruthy(leftOperand) && isTruthy(rightOperand), false};
                    break;
                }
                case DIV: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (isNumeric(rightOperand) && isNumeric(leftOperand)) {
                        int a = numericToInt(leftOperand), b = numericToInt(rightOperand);
                        if (b != 0) result = {"int", a / b, false};
                    }
                    break;
                }
                case MODULO: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (isNumeric(rightOperand) && isNumeric(leftOperand)) {
                        int a = numericToInt(leftOperand), b = numericToInt(rightOperand);
                        result = {"int", a % b, false};
                    }
                    break;
                }
                case EXPONENT: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (isNumeric(rightOperand) && isNumeric(leftOperand)) {
                        if (rightOperand.type == "float" || leftOperand.type == "float") {
                            result = {"float", std::pow(numericToFloat(leftOperand), numericToFloat(rightOperand)), false};
                            break;
                        }
                        int base = numericToInt(leftOperand), exponent = numericToInt(rightOperand);
                        if (exponent == 0) base = 1;
                        while(exponent-- > 0) {
                            base *= base;
                        }
                        result = {"int", base, false};
                    }
                    break;
                }
                case BIGGER_THAN:
                case BIGGER_OR_EQUAL:
                case SMALLER_THAN:
                case SMALLER_OR_EQUAL: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    bool val = firstIsBigger(rightOperand, leftOperand);
                    if (token.kind == SMALLER_OR_EQUAL || token.kind == SMALLER_THAN) val = not val;
                    if (token.kind == SMALLER_OR_EQUAL || token.kind == BIGGER_OR_EQUAL) val = val || compareValues(leftOperand, rightOperand);
                    result = {"bool", val, false};
                    break;
                }

                case COMPARATIVE_NOT_EQUALS:
                case COMPARATIVE_EQUALS: {
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    bool val = compareValues(rightOperand, leftOperand);
                    if (token.kind == COMPARATIVE_NOT_EQUALS) val = not val;
                    result = {"bool", val, false};
                    break;
                }
                case NOT: {
                    if (valueStack.empty()) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    result = {"bool", not isTruthy(rightOperand), false};
                    break;
                }
                case PLUS:
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (leftOperand.type == "string" && rightOperand.type == "string") {
                        std::string res = stringify(leftOperand)
                                + stringify(rightOperand);
                        result = {"string", res, false};
                    } else if ((leftOperand.type == "string" || rightOperand.type == "string")) {
                        std::string res = stringify(leftOperand) + stringify(rightOperand);
                        result = {"string", res, false};
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
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if (leftOperand.type == "string" && rightOperand.type == "string") {
                        std::string left = stringify(leftOperand), right = stringify(rightOperand);
                        size_t pos = left.find(right);
                        while(pos != std::string::npos) {
                            left.replace(pos, right.length(), "");
                            pos = left.find(right);
                        }
                        result = {"string", left, false};
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
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
                    if ((leftOperand.type == "string" || rightOperand.type == "string")
                    &&  (isNumeric(leftOperand) || (isNumeric(rightOperand)))) {
                        int times; std::string str;
                        if (leftOperand.type == "string") { str = stringify(leftOperand); times = numericToInt(rightOperand); }
                        else { str = stringify(rightOperand); times = numericToInt(leftOperand); }
                        std::string res;
                        while (times > 0) {
                            res += str; times--;
                        }
                        result = {"string", res, false};
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
                    if (valueStack.size() < 2) break;
                    rightOperand = valueStack.top();
                    valueStack.pop();
                    leftOperand = valueStack.top();
                    valueStack.pop();
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

    if (!valueStack.empty()) {
        return valueStack.top();
    }
    return {"null"};
}

// Function to convert an infix expression to Reverse Polish Notation (RPN)
TokenList Compiler::parseExpression() {
    std::vector<Token> output;
    std::stack<Token> operatorStack;

    while (lexer.hasNextToken()) {
        Token token = lexer.nextToken();
        Token nextToken = lexer.peekToken();

        if (token.kind == IDENTIFIER) {
            if (nextToken.kind == LEFT_BRACE) {
                lexer.nextToken();
                int argCount = 0;
                while(true) {
                    if (lexer.peekToken().kind == RIGHT_BRACE) { //print("Hello", "there", print(5), print(6), 7);
                        lexer.nextToken();
                        break;
                    }
                    TokenList expression = parseExpression();
                    output.insert(output.end(), expression.begin(), expression.end());
                    argCount++;
                    if (lexer.peekToken().kind == COMMA) {
                        lexer.nextToken();
                        continue;
                    }
                }
                output.push_back({INTEGER, std::to_string(argCount)});
            }
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
            bool mine = false; std::stack<Token> copyOperator = operatorStack;
            while(!operatorStack.empty()) {
                if (copyOperator.top().kind == LEFT_BRACE) {
                    mine = true;
                    break;
                }
                copyOperator.pop();
           }
            if (!mine) {
                lexer.moveReader(-1);
                break;
            }
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