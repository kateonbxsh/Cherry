#pragma once

#include "data.h"
#include "lexer.h"


bool isOperator(const Token& token);
bool isTruthy(const Value& value);
int precedence(const Token& token);
std::string stringify(const Value& value);