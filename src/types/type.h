#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "macros.h"
#include "function.h"

struct Value;
class Expression;
class Type;

struct Method {
    unsigned int flags = 0;
    std::vector<Function> overloads;
};

enum MemberFlags {
    Public = 1,
    Private = 2,
    Protected = 4,
    Static = 8,
    Readonly = 16
};

struct Field {
    std::string name;
    reference<Expression> type;
    unsigned int flags;
    bool hasDefaultValue;
    reference<Expression> value;
};

struct TypeParameter {
    string name;
    reference<Type> value;
    bool hasDefault;
    reference<Type> defaultValue;
};

class ClassInstance {

public:
    reference<Type> classType;
    std::unordered_map<string, Value> fieldValues;

};

enum TypeKind {
    Primitive,
    Dynamic,
    Class,
};

class Type {

public:

    static void defineTypes();

    Type(): kind(TypeKind::Primitive) {};
    explicit Type(TypeKind kind): kind(kind) {};

    bool assignableFrom(const Value& other);

    void setName(const string& name);
    string getName();

    void setDefaultValue(const Value& value);
    Value getDefaultValue();

    TypeKind kind;

    std::vector<TypeParameter> typeParameters;

    reference<Value> defaultValue;

    reference<Type> parent;

    Method constructor;
    std::unordered_map<string, Method> methods;
    std::unordered_map<string, Method> staticMethods;
    std::vector<Field> fields;
    std::vector<Field> staticFields;
    std::unordered_map<string, reference<Value>> staticFieldValues;
    std::unordered_map<string, reference<Type>> typeBindings;

private:

    string name;

};

extern reference<Type> IntegerType;
extern reference<Type> RealType;
extern reference<Type> StringType;
extern reference<Type> BooleanType;
extern reference<Type> FunctionType;
extern reference<Type> TypeType;
extern reference<Type> AnonymousType;
