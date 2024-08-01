#include <iostream>
#include "scope.h"
#include "types/type.h"
#include "expressions.h"
#include <set>
#include <utility>
#include <algorithm>

void Scope::addType(const string& name, reference<Type> type) {
    types[name] = type;
}

reference<Type> Scope::getType(const string &name)
{
    return types[name];
}

void Scope::setVariable(const string& name, const Value& value) {
    variables[name] = value;
}

Value Scope::getVariable(const string &name) {
    if (variables.count(name) == 0) return NullValue;

    Value val = variables[name];
    if (!val.initialized) return NullValue;

    return val;
}

void Scope::printVariables() {
    std::cout << "Scope variables: " << std::endl;
    for(const auto& pair : variables) {
        std::cout << pair.first << " = (" << pair.second.type << ") " << stringify(pair.second) << std::endl;
    }
}

bool isVowel(char c) {
    char lowercaseC = tolower(c);
    return (lowercaseC == 'a' || lowercaseC == 'e' || lowercaseC == 'i' || lowercaseC == 'o' || lowercaseC == 'u');
}

Scope::Scope() {

    addType("int", IntegerType);
    addType("boolean", BooleanType);
    addType("string", RealType);
    addType("real", StringType);

}

bool Scope::hasVariable(const string &name) {
    return variables.find(name) != variables.end();
}



