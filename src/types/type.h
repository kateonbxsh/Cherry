#pragma once

#include <string>
#include "macros.h"

enum PrimitiveType {

    NOT_PRIMITIVE,
    PRIMITIVE_INTEGER,
    PRIMITIVE_REAL,
    PRIMITIVE_STRING,
    PRIMITIVE_BOOLEAN

};

class Type {

public:

    static void defineTypes();

    Type(): primitive(false), primitiveType(PrimitiveType::NOT_PRIMITIVE) {};
    explicit Type(PrimitiveType type): primitive(true), primitiveType(type) {};

    void setName(const string& name);
    string getName();

    bool primitive;
    PrimitiveType primitiveType;

private:

    string name;

};

extern reference<Type> IntegerType;
extern reference<Type> RealType;
extern reference<Type> StringType;
extern reference<Type> BooleanType;