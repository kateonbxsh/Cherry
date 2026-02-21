#pragma once

#include <string>
#include "macros.h"
#include "class.h"

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

    virtual bool assignableFrom(const Value& other);

    void setName(const string& name);
    string getName();

    void setDefaultValue(const Value& value);
    Value getDefaultValue();

    TypeKind kind;

    std::vector<TypeParameter> typeParameters;

    Value defaultValue;

    reference<Type> parent;

    Method constructors;
    std::unordered_map<string, Method> methods;
    std::vector<Field> fields;

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