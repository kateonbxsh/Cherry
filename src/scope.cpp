#include <iostream>
#include "scope.h"
#include "type.h"
#include "expression.h"
#include <set>
#include <utility>
#include <algorithm>

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

Value internalPrint(std::vector<Value>& args) {
    for(Value& arg : args) {
        std::cout << stringify(arg) << " ";
    }
    std::cout << "\n";
    return {"null"};
}
bool isVowel(char c) {
    char lowercaseC = tolower(c);
    return (lowercaseC == 'a' || lowercaseC == 'e' || lowercaseC == 'i' || lowercaseC == 'o' || lowercaseC == 'u');
}
Value internalLinda(std::vector<Value>& args) {
    if (args.empty()) return {"null"};
    std::string strVal = stringify(args[0]);
    strVal.erase(std::remove_if(strVal.begin(), strVal.end(), isVowel), strVal.end());
    return {"string", strVal};
}

Scope::Scope() {

    static Type
        typeInt("int", 0),
        typeBool("bool", false),
        typeStr("string", ""),
        typeFloat("float", 0.0f);

    addType(typeInt);
    addType(typeStr);
    addType(typeFloat);
    addType(typeBool);

    Method iPrint; iPrint.internal = true; iPrint.internalMethod = internalPrint;
    setMethod("print", iPrint);
    Method iLinda; iLinda.internal = true; iLinda.internalMethod = internalLinda;
    setMethod("linda", iLinda);

}

std::vector<Type> Scope::getTypes() {
    return types;
}

void Scope::setMethod(const std::string &name, Method method) {
    methods[name] = std::move(method);
}

bool Scope::hasMethod(const std::string &name) {
    return methods.find(name) != methods.end();
}

Method Scope::getMethod(const std::string &name) {
    return methods[name];
}

bool Scope::hasVariable(const std::string &name) {
    return variables.find(name) != variables.end();
}



