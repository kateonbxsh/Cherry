#pragma once

#include "data.h"
#include "lexer.h"


bool isOperator(const Token& token);
bool isTruthy(const Value& value);
int precedence(const Token& token);
string stringify(const Value& value);

Value performOperator(const Value& value1, const Value& value2, TokenKind op);

boolean isNumeric(const Value& value);
boolean areNumericTypes(const Value& value1, const Value& value2);
real getNumericValueAsReal(const Value& value);
integer getNumericValueAsInt(const Value& value);

reference<Type> getResultType(const Value& value1, const Value& value2);

Value add(const Value& value1, const Value& value2);
Value subtract(const Value& value1, const Value& value2);
Value multiply(const Value& value1, const Value& value2);
Value divide(const Value& value1, const Value& value2);
Value exponent(const Value& value1, const Value& value2);
