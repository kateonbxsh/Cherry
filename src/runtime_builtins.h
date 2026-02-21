#pragma once

#include <unordered_map>
#include <vector>
#include "data.h"

class Scope;

void registerBuiltinRuntime(Scope& scope);
Value makeArrayFromValues(const std::vector<Value>& values);
bool tryGetArrayItems(Value& value, std::vector<Value>*& out);
bool tryGetMapEntries(Value& value, std::unordered_map<std::string, Value>*& out);
reference<Type> getArrayTypeBuiltin();
void initializeBuiltinInstance(ClassInstance& instance);
