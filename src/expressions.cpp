#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <utility>
#include <valarray>
#include "parser.h"
#include <cmath>

bool isNumeric(const Value& value) {
    return value.type == RealType ||
           value.type == IntegerType ||
           value.type == BooleanType;
}

bool areNumericTypes(const Value& value1, const Value& value2) {
    return isNumeric(value1) && isNumeric(value2);
}

float getNumericValueAsReal(const Value& value) {

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
            return 0.0f;
    }
}

int getNumericValueAsInt(const Value& value) {

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

    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        float result = getNumericValueAsReal(value1) + getNumericValueAsReal(value2);
        return Value(RealType, result);
    }
    if (resultType == IntegerType) {
        int result = getNumericValueAsInt(value1) + getNumericValueAsInt(value2);
        return Value(IntegerType, result);
    }
    
    return Value();
}

Value subtract(const Value& value1, const Value& value2) {
    
    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        float result = getNumericValueAsReal(value1) - getNumericValueAsReal(value2);
        return Value(RealType, result);
    }
    if (resultType == IntegerType) {
        int result = getNumericValueAsInt(value1) - getNumericValueAsInt(value2);
        return Value(IntegerType, result);
    }
    
    return Value();
}

Value multiply(const Value& value1, const Value& value2) {
    
    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        float result = getNumericValueAsReal(value1) * getNumericValueAsReal(value2);
        return Value(RealType, result);
    }
    if (resultType == IntegerType) {
        int result = getNumericValueAsInt(value1) * getNumericValueAsInt(value2);
        return Value(IntegerType, result);
    }
    
    return Value();
}

Value divide(const Value& value1, const Value& value2) {
    
    if (!areNumericTypes(value1, value2)) return NullValue;

    reference<Type> resultType = getResultType(value1, value2);
    if (resultType == RealType) {
        real denominator = getNumericValueAsReal(value2);
        if (denominator == 0) {
            return NullValue;
        }
        real result = getNumericValueAsReal(value1) / denominator;
        return Value(RealType, result);
    }
    if (resultType == IntegerType) {
        int denominator = getNumericValueAsInt(value2);
        if (denominator == 0) {
            return NullValue;
        }
        int result = getNumericValueAsInt(value1) / denominator;
        return Value(IntegerType, result);
    }
    
    return Value();
}

Value exponent(const Value& value1, const Value& value2) {

    if (!areNumericTypes(value1, value2)) return NullValue;

    real base = getNumericValueAsReal(value1);
    real exp = getNumericValueAsReal(value2);

    return Value(RealType, std::pow(base, exp));
}
