#include <cmath>
#include <functional>
#include <string>
#include <unordered_map>
#include "parser.h"
#include "expressions.h"
#include "data.h"

using BinaryOp = std::function<Value(const Value&, const Value&)>;
using UnaryOp = std::function<Value(const Value&)>;

/* ────────────────────────────────────────────────────────────── *
 *  VALUE HELPERS
 * ────────────────────────────────────────────────────────────── */

template<typename T>
T getValue(const Value& value) {
    return std::get<T>(value.value);
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
    if (!isNumeric(value)) return 0;

    if (value.type == RealType) return std::get<real>(value.value);
    if (value.type == IntegerType) return static_cast<real>(std::get<integer>(value.value));
    if (value.type == BooleanType) return static_cast<real>(std::get<boolean>(value.value));
    return 0;
}

integer getNumericValueAsInt(const Value& value) {
    if (!isNumeric(value)) return 0;

    if (value.type == RealType) return static_cast<integer>(std::get<real>(value.value));
    if (value.type == IntegerType) return std::get<integer>(value.value);
    if (value.type == BooleanType) return static_cast<integer>(std::get<boolean>(value.value));
    return 0;
}

bool isTruthy(const Value& value) {

    if (value.type == nullptr) return false;
    if (value.type->kind != TypeKind::Primitive) return true;
    if (value.type == RealType) return getValue<real>(value) > 0.5f;
    if (value.type == StringType) return !getValue<string>(value).empty();
    if (value.type == IntegerType) return getValue<integer>(value) > 0;
    if (value.type == BooleanType) return getValue<boolean>(value);
    return false;
}

std::string stringify(const Value& value) {

    if (value.type == nullptr) return "null";
    if (value.type->kind != TypeKind::Primitive) return "<Object>";
    if (value.type == RealType) return std::to_string(getValue<real>(value));
    if (value.type == StringType) return getValue<string>(value);
    if (value.type == IntegerType) return std::to_string(getValue<integer>(value));
    if (value.type == BooleanType) return getValue<boolean>(value) ? "true" : "false";
    if (value.type == FunctionType) return "<function>";
    if (value.type == TypeType) {
        auto type = get<reference<Type>>(value.value);
        return "<type " + type->getName() + ">";
    }
    return "null";
}

boolean compareValues(const Value& value1, const Value& value2) {
    
    if (value1.type != value2.type && !areNumericTypes(value1, value2)) return false;
    if (value1.type == nullptr) return false;
    if (value1.type->kind != TypeKind::Primitive) return false;

    if (value1.type == RealType || value2.type == RealType)
        return getNumericValueAsReal(value1) == getNumericValueAsReal(value2);
    if (value1.type == IntegerType || value2.type == IntegerType)
        return getNumericValueAsInt(value1) == getNumericValueAsInt(value2);

    return getValue<boolean>(value1) == getValue<boolean>(value2);
}

boolean firstIsBigger(const Value& value1, const Value& value2) {

    if (value1.type != value2.type && !areNumericTypes(value1, value2)) return false;
    if (value1.type == nullptr) return false;
    if (value1.type->kind != TypeKind::Primitive) return false;

    if (value1.type == RealType || value2.type == RealType)
        return getNumericValueAsReal(value1) > getNumericValueAsReal(value2);
    if (value1.type == IntegerType || value2.type == IntegerType)
        return getNumericValueAsInt(value1) > getNumericValueAsInt(value2);

    return false;
}

/* ────────────────────────────────────────────────────────────── *
 *  OPERATOR ERROR FACTORY
 * ────────────────────────────────────────────────────────────── */

static Value op_error(const TokenKind op, const Value& a, const Value& b) {
    Value v;
    v.thrownException = create_reference<Value>(
        Value("operator '" + tokenKindStrings[op] + "' not supported between '" +
            (a.type ? a.type->getName() : "<unknown type>") + "' and '" +
            (b.type ? b.type->getName() : "<unknown type>") + "'")
    );
    return v;
}
static Value op_error(const TokenKind op, const Value& b) {
    Value v;
    v.thrownException = create_reference<Value>(
        Value("operator '" + tokenKindStrings[op] + "' not supported for type '" +
            (b.type ? b.type->getName() : "<unknown type>") + "'")
    );
    return v;
}

/* ────────────────────────────────────────────────────────────── *
 *  NUMERIC OPS
 * ────────────────────────────────────────────────────────────── */

namespace numeric_ops {

static Value add(const Value& a, const Value& b) {
    auto r = getNumericValueAsReal(a) + getNumericValueAsReal(b);
    if (a.type == RealType || b.type == RealType) return Value(real(r));
    return Value(integer(r));
}

static Value subtract(const Value& a, const Value& b) {
    auto r = getNumericValueAsReal(a) - getNumericValueAsReal(b);
    if (a.type == RealType || b.type == RealType) return Value(real(r));
    return Value(integer(r));
}

static Value multiply(const Value& a, const Value& b) {
    auto r = getNumericValueAsReal(a) * getNumericValueAsReal(b);
    if (a.type == RealType || b.type == RealType) return Value(real(r));
    return Value(integer(r));
}

static Value divide(const Value& a, const Value& b) {
    real d = getNumericValueAsReal(b);
    if (d == 0) return NullValue;
    return Value(real(getNumericValueAsReal(a) / d));
}

static Value exponent(const Value& a, const Value& b) {
    return Value(real(std::pow(getNumericValueAsReal(a), getNumericValueAsReal(b))));
}

static Value modulo(const Value& a, const Value& b) {
    integer x = getNumericValueAsInt(a);
    integer y = getNumericValueAsInt(b);
    if (y == 0) return NullValue;
    return Value(x % y);
}

static Value div(const Value& a, const Value& b) {
    integer x = getNumericValueAsInt(a);
    integer y = getNumericValueAsInt(b);
    if (y == 0) return NullValue;
    return Value(x / y);
}

static Value eq(const Value& a, const Value& b) {
    return Value(compareValues(a, b));
}

static Value neq(const Value& a, const Value& b) {
    return Value((boolean) !compareValues(a, b));
}

static Value gt(const Value& a, const Value& b) {
    return Value(firstIsBigger(a, b));
}

static Value lt(const Value& a, const Value& b) {
    return Value(firstIsBigger(b, a));
}

static Value ge(const Value& a, const Value& b) {
    return Value((boolean) !firstIsBigger(b, a));
}

static Value le(const Value& a, const Value& b) {
    return Value((boolean) !firstIsBigger(a, b));
}

static Value negative(const Value& a) {
    if (a.type == IntegerType)
        return Value(- getNumericValueAsInt(a));
    return Value(- getNumericValueAsReal(a));
}

static Value positive(const Value& a) {
    return a;
}

}

/* ────────────────────────────────────────────────────────────── *
 *  STRING OPS
 * ────────────────────────────────────────────────────────────── */

namespace string_ops {

static Value add(const Value& a, const Value& b) {
    return Value(stringify(a) + stringify(b));
}

static Value multiply(const Value& a, const Value& b) {
    integer n = getNumericValueAsInt(b);
    if (n <= 0) return Value(string(""));
    string result;
    result.reserve(stringify(a).size() * n);
    while (n--) result += stringify(a);
    return Value(result);
}

}

/* ────────────────────────────────────────────────────────────── *
 *  BOOLEAN OPS
 * ────────────────────────────────────────────────────────────── */

namespace boolean_ops {

    static Value logical_and(const Value& a, const Value& b) {
        return Value((boolean) (isTruthy(a) && isTruthy(b)));
    }

    static Value logical_or(const Value& a, const Value& b) {
        return Value((boolean)(isTruthy(a) || isTruthy(b)));
    }

    static Value logical_xor(const Value& a, const Value& b) {
        return Value((boolean)(isTruthy(a) != isTruthy(b)));
    }

    static Value logical_not(const Value& a) {
        return Value((boolean)(!isTruthy(a)));
    }

}

/* ────────────────────────────────────────────────────────────── *
 *  OP TABLE REGISTRATION
 * ────────────────────────────────────────────────────────────── */

static std::unordered_map<TokenKind, BinaryOp> numericBinaryTable = {
    {PLUS,        numeric_ops::add},
    {MINUS,       numeric_ops::subtract},
    {TIMES,       numeric_ops::multiply},
    {DIVIDE,      numeric_ops::divide},
    {EXPONENT,    numeric_ops::exponent},
    {MODULO,      numeric_ops::modulo},
    {DIV,         numeric_ops::div},
    {COMPARATIVE_EQUALS, numeric_ops::eq},
    {COMPARATIVE_NOT_EQUALS, numeric_ops::neq},
    {BIGGER_THAN, numeric_ops::gt},
    {SMALLER_THAN, numeric_ops::lt},
    {BIGGER_OR_EQUAL, numeric_ops::ge},
    {SMALLER_OR_EQUAL, numeric_ops::le},
};

static std::unordered_map<TokenKind, UnaryOp> numericUnaryTable = {
    {PLUS, numeric_ops::positive},
    {MINUS, numeric_ops::negative}
};


static std::unordered_map<TokenKind, BinaryOp> stringBinaryTable = {
    {PLUS, string_ops::add},
    {TIMES, string_ops::multiply},
};

static std::unordered_map<TokenKind, BinaryOp> booleanBinaryTable = {
    {AND, boolean_ops::logical_and},
    {OR, boolean_ops::logical_or},
    {XOR, boolean_ops::logical_xor},
};

static std::unordered_map<TokenKind, UnaryOp> booleanUnaryTable = {
    {NOT, boolean_ops::logical_not}
};

/* ────────────────────────────────────────────────────────────── *
 *  MAIN OP DISPATCH
 * ────────────────────────────────────────────────────────────── */

Value performUnaryOperator(const Value& a, TokenKind op) {
    if (booleanUnaryTable.count(op) && (a.type == BooleanType)) {
        return booleanUnaryTable[op](a);
    }
    if (numericUnaryTable.count(op) && (isNumeric(a))) {
        return numericUnaryTable[op](a);
    }
    return op_error(op, a);
}

Value performBinaryOperator(const Value& a, const Value& b, TokenKind op) {

    // boolean?
    if (booleanBinaryTable.count(op) && (a.type == BooleanType || b.type == BooleanType)) {
        return booleanBinaryTable[op](a, b);
    }

    // string?
    if (stringBinaryTable.count(op) && (a.type == StringType || b.type == StringType)) {
        return stringBinaryTable[op](a, b);
    }

    // numeric?
    if (numericBinaryTable.count(op) && areNumericTypes(a, b)) {
        return numericBinaryTable[op](a, b);
    }

    // fallback: error
    return op_error(op, a, b);
}

bool isBinaryOperator(const TokenKind& kind) {
    return (kind > BEGIN_OF_BINARY_OPERATORS && kind < END_OF_BINARY_OPERATORS);
}

bool isUnaryOperator(const TokenKind& kind) {
    return (kind > BEGIN_OF_UNARY_OPERATORS && kind < END_OF_UNARY_OPERATORS) || kind == MINUS || kind == PLUS;
}

int precedence(const Token& token) {
    TokenKind kind = token.kind;
    if (isUnaryOperator(kind)) return 5;
    if (kind == EXPONENT)       return 4;
    if (kind == DIVIDE)         return 3;
    if (kind == TIMES)          return 2;
    if (kind == PLUS || kind == MINUS) return 1;
    return 0;
}
