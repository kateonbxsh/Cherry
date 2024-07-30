#include <iostream>
#include "scope.h"
#include "types/type.h"
#include "expressions.h"
#include <set>
#include <utility>
#include <algorithm>

void Scope::addType(const std::string& name, Type* type) {
    types[name] = type;
}

Type *Scope::getType(const std::string &name)
{
    return types[name];
}

void Scope::setVariable(const std::string& name, const Value& value) {
    variables[name] = value;
}

Value Scope::getVariable(const std::string &name) {
    if (variables.count(name) > 0) {
        return variables[name];
    }
    return NullValue;
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

    addType("int", (Type*) IntegerType);
    addType("boolean", (Type*) BooleanType);
    addType("string", (Type*) FloatType);
    addType("float", (Type*) StringType);

}

bool Scope::hasVariable(const std::string &name) {
    return variables.find(name) != variables.end();
}



