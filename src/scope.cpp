#include <iostream>
#include "scope.h"
#include "type.h"
#include "expression.h"
#include <set>

void Scope::addType(const Type& type) {
    types.push_back(type);
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

Scope::Scope() {

    static Type
        typeInt("int", 0),
        typeBool("bool", false),
        typeStr("str", ""),
        typeFloat("float", 0.0f);

    addType(typeInt);
    addType(typeStr);
    addType(typeFloat);
    addType(typeBool);

}

std::vector<Type> Scope::getTypes() {
    return types;
}

