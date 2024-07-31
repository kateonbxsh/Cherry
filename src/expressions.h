#pragma once

#include "data.h"
#include "lexer.h"


bool isOperator(const Token& token);
bool isTruthy(const Value& value);
int precedence(const Token& token);
string stringify(const Value& value);

Value performOperator(const Value& value1, const Value& value2, TokenKind op);