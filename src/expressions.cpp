#include <cmath>
#include <functional>
#include <string>
#include <unordered_map>
#include "parser.h"
#include "expressions.h"
#include "data.h"
#include "scope.h"
#include "statements/Block.hpp"

using BinaryOp = std::function<Value(const Value&, const Value&)>;
using UnaryOp = std::function<Value(const Value&)>;

static std::string getOperatorMethodName(TokenKind op) {
    switch (op) {
        case PLUS: return "operator+";
        case MINUS: return "operator-";
        case TIMES: return "operator*";
        case DIVIDE: return "operator/";
        case EXPONENT: return "operator^";
        case MODULO: return "operatormod";
        case DIV: return "operatordiv";
        case COMPARATIVE_EQUALS: return "operator==";
        case COMPARATIVE_NOT_EQUALS: return "operator!=";
        case BIGGER_THAN: return "operator>";
        case BIGGER_OR_EQUAL: return "operator>=";
        case SMALLER_THAN: return "operator<";
        case SMALLER_OR_EQUAL: return "operator<=";
        case AND: return "operatorand";
        case OR: return "operatoror";
        case XOR: return "operatorxor";
        case BITWISE_AND: return "operator&";
        case BITWISE_OR: return "operator|";
        case BITWISE_XOR: return "operator!|";
        default: return "";
    }
}

static Value invokeOverload(
    const Function& fn,
    const std::vector<Value>& args,
    const reference<Type>& ownerType = nullptr
) {
    if (fn.kind == FunctionKind::Internal) {
        Scope internalScope(fn.closure);
        if (!fn.internalHandler) {
            Value err;
            err.thrownException = create_reference<Value>(Value("internal function is missing implementation"));
            return err;
        }
        return fn.internalHandler(internalScope, args, fn.__this);
    }

    Scope funcScope(fn.closure);
    if (ownerType != nullptr && !ownerType->getName().empty()) {
        auto ownerTypeCopy = ownerType;
        funcScope.addVariable(ownerType->getName(), Value(ownerTypeCopy));
    }
    if (fn.__this != nullptr) {
        funcScope.addVariable("this", *fn.__this);
    }
    for (size_t i = 0; i < fn.parameters.size(); ++i) {
        funcScope.addVariable(fn.parameters[i].name, args[i]);
    }
    return fn.body->execute(funcScope);
}

struct ClassOperatorResult {
    bool handled = false;
    Value value = NullValue;
};

static ClassOperatorResult tryClassBinaryOperator(const Value& a, const Value& b, TokenKind op) {
    auto methodName = getOperatorMethodName(op);
    if (methodName.empty()) return {};

    std::vector<reference<Type>> candidates;
    if (a.type != nullptr && a.type->kind == TypeKind::Class) candidates.push_back(a.type);
    if (b.type != nullptr && b.type->kind == TypeKind::Class && b.type != a.type) candidates.push_back(b.type);

    std::vector<Value> args = {a, b};
    for (const auto& ownerType : candidates) {
        if (!ownerType->staticMethods.contains(methodName)) continue;
        const auto& method = ownerType->staticMethods.at(methodName);
        for (const auto& overload : method.overloads) {
            if (!overload.validArguments(args)) continue;
            return {true, invokeOverload(overload, args, ownerType)};
        }
    }

    return {};
}

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
    auto stringifyClassName = [](const reference<Type>& type) -> string {
        if (type == nullptr) return "<unknown>";
        string out = type->getName();

        reference<Type> root = type;
        while (root->parent != nullptr) {
            root = root->parent;
        }

        const auto& displayParams = root->typeParameters;
        if (!displayParams.empty()) {
            out += "<";
            for (size_t i = 0; i < displayParams.size(); ++i) {
                const auto& tp = displayParams[i];
                if (type->typeBindings.contains(tp.name) && type->typeBindings.at(tp.name) != nullptr) {
                    out += type->typeBindings.at(tp.name)->getName();
                } else if (tp.hasDefault && tp.defaultValue != nullptr) {
                    out += "? = " + tp.defaultValue->getName();
                } else {
                    out += "?";
                }

                if (i + 1 < displayParams.size()) out += ", ";
            }
            out += ">";
        }

        return out;
    };

    if (value.type == nullptr) return "null";
    if (value.type == RealType) return std::to_string(getValue<real>(value));
    if (value.type == StringType) return getValue<string>(value);
    if (value.type == IntegerType) return std::to_string(getValue<integer>(value));
    if (value.type == BooleanType) return getValue<boolean>(value) ? "true" : "false";
    if (value.type == FunctionType) return "[function]";
    if (value.type == TypeType) {
        auto type = get<reference<Type>>(value.value);
        if (type != nullptr && type->kind == TypeKind::Class) {
            return "[type " + stringifyClassName(type) + "]";
        }
        return "[type " + type->getName() + "]";
    }
    if (value.type->kind == TypeKind::Class) {
        reference<Type> cursor = value.type;
        while (cursor != nullptr) {
            if (cursor->methods.contains("display")) {
                const auto& method = cursor->methods.at("display");
                for (const auto& overload : method.overloads) {
                    if (!overload.parameters.empty()) continue;
                    Function fn = overload;
                    fn.__this = create_reference<Value>(value);
                    Value rendered;
                    if (fn.kind == FunctionKind::Internal) {
                        Scope internalScope(fn.closure);
                        if (!fn.internalHandler) break;
                        rendered = fn.internalHandler(internalScope, {}, fn.__this);
                    } else {
                        Scope funcScope(fn.closure);
                        if (fn.__this != nullptr) {
                            funcScope.addVariable("this", *fn.__this);
                        }
                        rendered = fn.body->execute(funcScope);
                    }
                    if (rendered.thrownException == nullptr && rendered.type == StringType) {
                        return get<string>(rendered.value);
                    }
                    break;
                }
                break;
            }
            cursor = cursor->parent;
        }
        return "[" + stringifyClassName(value.type) + " instance]";
    }

    return "[object]";
}

boolean compareValues(const Value& value1, const Value& value2) {
    
    if (value1.type != value2.type && !areNumericTypes(value1, value2)) return false;
    if (value1.type == nullptr) return value2.type == nullptr;
    if (value1.type == TypeType && value2.type == TypeType) {
        auto t1 = get<reference<Type>>(value1.value);
        auto t2 = get<reference<Type>>(value2.value);
        if (t1 == t2) return true;
        if (t1 == nullptr || t2 == nullptr) return false;
        if (t1->kind != t2->kind) return false;
        if (t1->getName() != t2->getName()) return false;

        if (t1->typeBindings.size() != t2->typeBindings.size()) return false;
        for (const auto& [name, bound1] : t1->typeBindings) {
            if (!t2->typeBindings.contains(name)) return false;
            auto bound2 = t2->typeBindings.at(name);
            if (bound1 == bound2) continue;
            if (bound1 == nullptr || bound2 == nullptr) return false;
            if (bound1->getName() != bound2->getName()) return false;
        }
        return true;
    }
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
    if (op == COMPARATIVE_EQUALS) {
        if (a.type == nullptr || b.type == nullptr) {
            return Value((boolean)(a.type == b.type));
        }
    }
    if (op == COMPARATIVE_NOT_EQUALS) {
        if (a.type == nullptr || b.type == nullptr) {
            return Value((boolean)(a.type != b.type));
        }
    }

    if (op == IS) {
        if (b.type != TypeType) {
            return op_error(op, a, b);
        }
        auto checkType = get<reference<Type>>(b.value);
        return Value((boolean)checkType->assignableFrom(a));
    }

    ClassOperatorResult classOperatorResult = tryClassBinaryOperator(a, b, op);
    if (classOperatorResult.handled) {
        return classOperatorResult.value;
    }

    if (op == COMPARATIVE_EQUALS) {
        return Value((boolean)compareValues(a, b));
    }
    if (op == COMPARATIVE_NOT_EQUALS) {
        return Value((boolean)!compareValues(a, b));
    }

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
    return (kind > BEGIN_OF_UNARY_OPERATORS && kind < END_OF_UNARY_OPERATORS) || kind == MINUS || kind == PLUS || kind == TYPEOF;
}

int precedence(const Token& token) {
    TokenKind kind = token.kind;
    if (kind == EXPONENT) return 7;
    if (kind == TIMES || kind == DIVIDE || kind == MODULO || kind == DIV) return 6;
    if (kind == PLUS || kind == MINUS) return 5;
    if (kind == BIGGER_THAN || kind == BIGGER_OR_EQUAL ||
        kind == SMALLER_THAN || kind == SMALLER_OR_EQUAL) return 4;
    if (kind == COMPARATIVE_EQUALS || kind == COMPARATIVE_NOT_EQUALS || kind == IS) return 3;
    if (kind == BITWISE_AND || kind == BITWISE_OR || kind == BITWISE_XOR) return 2;
    if (kind == AND || kind == OR || kind == XOR) return 1;
    return 0;
}
