#include <iostream>
#include "scope.h"
#include "type.h"
#include "expressions.h"
#include <set>
#include <utility>
#include <algorithm>

void Scope::addType(const std::string& name, const Type& type) {
    types[name] = type;
}

void Scope::setVariable(const std::string& name, const Value& value) {
    variables[name] = value;
}

Value Scope::getVariable(const std::string &name) {
    if (variables.count(name) > 0) {
        return variables[name];
    }
    return {"null", 0};
}

void Scope::printVariables() {
    std::cout << "DECLARED VARIABLES: " << std::endl;
    for(const auto& pair : variables) {
        std::cout << pair.first << " = (" << pair.second.type << ") " << stringify(pair.second) << std::endl;
    }
}

bool isVowel(char c) {
    char lowercaseC = tolower(c);
    return (lowercaseC == 'a' || lowercaseC == 'e' || lowercaseC == 'i' || lowercaseC == 'o' || lowercaseC == 'u');
}

Scope::Scope() {

    static Type
        typeInt,
        typeBool,
        typeStr,
        typeFloat;

    addType("int", typeInt);
    addType("bool", typeStr);
    addType("str", typeFloat);
    addType("float", typeBool);

}

bool Scope::hasVariable(const std::string &name) {
    return variables.find(name) != variables.end();
}



