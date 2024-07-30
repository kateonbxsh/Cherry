#pragma once
#include <string>

enum PrimitiveType {

    NOT_PRIMITIVE,
    PRIMITIVE_INTEGER,
    PRIMITIVE_FLOAT,
    PRIMITIVE_STRING,
    PRIMITIVE_BOOLEAN

};

class Type {

public:

    static void defineTypes();

    Type(): primitive(false), primitiveType(PrimitiveType::NOT_PRIMITIVE) {};
    explicit Type(PrimitiveType type): primitive(true), primitiveType(type) {};

    void setName(const std::string& name);
    std::string getName();

    bool primitive;
    PrimitiveType primitiveType;

private:

    std::string name;

};

extern Type* IntegerType;
extern Type* FloatType;
extern Type* StringType;
extern Type* BooleanType;