#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <utility>
#include <valarray>
#include "parser.h"
#include "executor.h"

bool firstIsBigger(const Value& value1, const Value& value2) {

    if (value1.type != value2.type) return false;

    if (value1.type->primitiveType == PRIMITIVE_FLOAT) return *((float*) (value1.value)) > *((float*) (value2.value));
    if (value1.type->primitiveType == PRIMITIVE_STRING) return *((std::string*) (value1.value)) > *((std::string*) (value2.value));
    if (value1.type->primitiveType == PRIMITIVE_INTEGER) return *((int*) (value1.value)) > *((int*) (value2.value));
    if (value1.type->primitiveType == PRIMITIVE_BOOLEAN) return *((bool*) (value1.value)) > *((bool*) (value2.value));

    return false;
}

bool compareValues(const Value& value1, const Value& value2) {

    if (value1.type != value2.type) return false;

    if (value1.type->primitiveType == PRIMITIVE_FLOAT) return *((float*) (value1.value)) == *((float*) (value2.value));
    if (value1.type->primitiveType == PRIMITIVE_STRING) return *((std::string*) (value1.value)) == *((std::string*) (value2.value));
    if (value1.type->primitiveType == PRIMITIVE_INTEGER) return *((int*) (value1.value)) == *((int*) (value2.value));
    if (value1.type->primitiveType == PRIMITIVE_BOOLEAN) return *((bool*) (value1.value)) == *((bool*) (value2.value));
    
    return false;
}

bool isTruthy(const Value& value) {
    if (!value.type->primitive) return (value.value != nullptr);
    if (value.type->primitiveType == PRIMITIVE_FLOAT) return *((float*) (value.value)) > 0.5f;
    if (value.type->primitiveType == PRIMITIVE_STRING) return (*((std::string*) (value.value))).length() > 0;
    if (value.type->primitiveType == PRIMITIVE_INTEGER) return *((int*) (value.value)) > 0;
    if (value.type->primitiveType == PRIMITIVE_BOOLEAN) return *((bool*) (value.value));
    return false;
}
bool isNumeric(const Value& value) {
    return (
            value.type->primitiveType == PRIMITIVE_FLOAT ||
        value.type->primitiveType == PRIMITIVE_INTEGER ||
                    value.type->primitiveType == PRIMITIVE_BOOLEAN
    );
}
std::string stringify(const Value& value) {
    if (value.type->primitiveType == PRIMITIVE_STRING) return *((std::string*) (value.value));
    if (value.type->primitiveType == PRIMITIVE_INTEGER) return std::to_string(*((int*) (value.value)));
    if (value.type->primitiveType == PRIMITIVE_FLOAT) return std::to_string(*((float*) (value.value)));
    if (value.type->primitiveType == PRIMITIVE_BOOLEAN) return std::to_string(*((bool*) (value.value)));
    return "<object>";
}

bool isOperator(const Token& token) {
    TokenKind kind = token.kind;
    return (kind > BEGIN_OF_OPERATORS && kind < END_OF_OPERATORS);
}

int precedence(const Token& token) {
    TokenKind kind = token.kind;
    if (kind == AND || kind == OR) {
        return 3;
    } else if (kind == TIMES || kind == DIVIDE) {
        return 2;
    } else if (kind == PLUS || kind == MINUS) {
        return 1;
    }
    return kind != LEFT_BRACE;
}