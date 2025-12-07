#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <utility>
#include <valarray>
#include "parser.h"
#include "expressions.h"
#include <cmath>

template<typename T>
T getValue(const Value& value) {
    return std::get<T>(value.value);
}

boolean firstIsBigger(const Value& value1, const Value& value2) {

    if (value1.type != value2.type && !areNumericTypes(value1, value2)) return false;
    if (value1.type == nullptr) return false;
    if (!value1.type->primitive) return false;

    if (value1.type == RealType || value2.type == RealType)
            return getNumericValueAsReal(value1) > getNumericValueAsReal(value2);
    if (value1.type == IntegerType || value2.type == IntegerType)
            return getNumericValueAsInt(value1) > getNumericValueAsInt(value2);

    return false;

}

boolean compareValues(const Value& value1, const Value& value2) {

    if (value1.type != value2.type && !areNumericTypes(value1, value2)) return false;
    if (value1.type == nullptr) return false;
    if (!value1.type->primitive) return false;

    if (value1.type == RealType || value2.type == RealType)
            return getNumericValueAsReal(value1) == getNumericValueAsReal(value2);
    if (value1.type == IntegerType || value2.type == IntegerType)
            return getNumericValueAsInt(value1) == getNumericValueAsInt(value2);

    return getValue<boolean>(value1) == getValue<boolean>(value2);
}

bool isTruthy(const Value& value) {

    if (value.type == nullptr) return false;
    if (!value.type->primitive) return true;

    switch (value.type->primitiveType) {
        case PRIMITIVE_REAL:
            return getValue<real>(value) > 0.5f;
        case PRIMITIVE_STRING:
            return !getValue<string>(value).empty();
        case PRIMITIVE_INTEGER:
            return getValue<integer>(value) > 0;
        case PRIMITIVE_BOOLEAN:
            return getValue<boolean>(value);
        default:
            return false;
    }
}

std::string stringify(const Value& value) {

    if (value.type == nullptr) return "null";
    if (!value.type->primitive) return "<Object>";

    switch (value.type->primitiveType) {
        case PRIMITIVE_REAL:
            return std::to_string(getValue<real>(value));
        case PRIMITIVE_STRING:
            return getValue<string>(value);
        case PRIMITIVE_INTEGER:
            return std::to_string(getValue<integer>(value));
        case PRIMITIVE_BOOLEAN:
            return getValue<boolean>(value) ? "true" : "false";
        default:
            return "null";
    }
}


boolean isNumeric(const Value& value) {
    return value.type == RealType ||
           value.type == IntegerType ||
           value.type == BooleanType;
}

boolean areNumericTypes(const Value& value1, const Value& value2) {
    return isNumeric(value1) && isNumeric(value2);
}

real getNumericValueAsReal(const Value& value) {

    if (!isNumeric(value)) {
        return 0;
    }

    switch (value.type->primitiveType) {
        case PRIMITIVE_REAL:
            return std::get<real>(value.value);
        case PRIMITIVE_INTEGER:
            return static_cast<real>(std::get<integer>(value.value));
        case PRIMITIVE_BOOLEAN:
            return static_cast<real>(std::get<boolean>(value.value));
        default:
            return 0;
    }
}

integer getNumericValueAsInt(const Value& value) {

    if (!isNumeric(value)) {
        return 0;
    }

    switch (value.type->primitiveType) {
        case PRIMITIVE_REAL:
            return static_cast<integer>(std::get<real>(value.value));
        case PRIMITIVE_INTEGER:
            return std::get<integer>(value.value);
        case PRIMITIVE_BOOLEAN:
            return static_cast<integer>(std::get<boolean>(value.value));
        default:
            return 0;
    }
}

reference<Type> getResultType(const Value& value1, const Value& value2) {
    if (value1.type == RealType || value2.type == RealType)
        return RealType;
    return IntegerType;
}

Value add(const Value& value1, const Value& value2) {

    if (value1.type == StringType || value2.type == StringType) {
        return Value(stringify(value1) + stringify(value2));
    }
    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        real result = getNumericValueAsReal(value1) + getNumericValueAsReal(value2);
        return Value(result);
    }
    if (resultType == IntegerType) {
        integer result = getNumericValueAsInt(value1) + getNumericValueAsInt(value2);
        return Value(result);
    }
    
    return Value();
}

Value subtract(const Value& value1, const Value& value2) {
    
    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        real result = getNumericValueAsReal(value1) - getNumericValueAsReal(value2);
        return Value(result);
    }
    if (resultType == IntegerType) {
        integer result = getNumericValueAsInt(value1) - getNumericValueAsInt(value2);
        return Value(result);
    }
    
    return NullValue;
}

Value multiply(const Value& value1, const Value& value2) {
    
    if (value1.type == StringType && isNumeric(value2)) {
        string result = "";
        int times = getNumericValueAsInt(value2);
        while (times--) result += stringify(value1);
        return Value(result);
    }
    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        real result = getNumericValueAsReal(value1) * getNumericValueAsReal(value2);
        return Value(result);
    }
    if (resultType == IntegerType) {
        integer result = getNumericValueAsInt(value1) * getNumericValueAsInt(value2);
        return Value(result);
    }
    
    return NullValue;
}

Value divide(const Value& value1, const Value& value2) {
    
    if (!areNumericTypes(value1, value2)) return NullValue;

    real denominator = getNumericValueAsReal(value2);
    if (denominator == 0) {
        return NullValue;
    }
    real result = getNumericValueAsReal(value1) / denominator;
    return Value(result);
}

Value exponent(const Value& value1, const Value& value2) {

    if (!areNumericTypes(value1, value2)) return NullValue;

    real base = getNumericValueAsReal(value1);
    real exp = getNumericValueAsReal(value2);

    return Value((real) std::pow(base, exp));
}

bool isOperator(const Token& token) {
    TokenKind kind = token.kind;
    return (kind > BEGIN_OF_OPERATORS && kind < END_OF_OPERATORS);
}

bool isOperator(const TokenKind& kind) {
    return (kind > BEGIN_OF_OPERATORS && kind < END_OF_OPERATORS);
}


int precedence(const Token& token) {
    TokenKind kind = token.kind;
    if (kind == EXPONENT) {
        return 4;
    } else if (kind == DIVIDE) {
        return 3;
    } else if (kind == TIMES) {
        return 2;
    } else if (kind == PLUS || kind == MINUS) {
        return 1;
    }
    return 0;
}

Value performOperator(const Value& value1, const Value& value2, TokenKind op) {
    if (op <= BEGIN_OF_OPERATORS || op >= END_OF_OPERATORS) return NullValue;
    if (value1.type == nullptr || value2.type == nullptr) return NullValue;

    switch (op)
    {
    case COMPARATIVE_EQUALS:
        return Value(compareValues(value1, value2));
    case COMPARATIVE_NOT_EQUALS:
        return Value((boolean) !compareValues(value1, value2));
    case BIGGER_THAN:
        return Value(firstIsBigger(value1, value2));
    case SMALLER_THAN:
        return Value(firstIsBigger(value2, value1));
    case BIGGER_OR_EQUAL:
        return Value((boolean) (!firstIsBigger(value2, value1)));
    case SMALLER_OR_EQUAL:
        return Value((boolean) (!firstIsBigger(value1, value2)));
    case PLUS:
        return add(value1, value2);
    case MINUS:
        return subtract(value1, value2);
    case TIMES:
        return multiply(value1, value2);
    case DIVIDE:
        return divide(value1, value2);
    case EXPONENT:
        return exponent(value1, value2);
    default:
        return NullValue;   
    }

}