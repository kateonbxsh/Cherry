#pragma once

enum PrimitiveType {

    NOT_PRIMITIVE,
    PRIMITIVE_INTEGER,
    PRIMITIVE_FLOAT,
    PRIMITIVE_STRING,
    PRIMITIVE_BOOLEAN

};

class Type {

public:

    Type(): primitive(false), primitiveType(PrimitiveType::NOT_PRIMITIVE) {};
    explicit Type(PrimitiveType type): primitive(true), primitiveType(type) {};

    bool primitive;
    PrimitiveType primitiveType;

};

extern const Type* IntegerType;
extern const Type* FloatType;
extern const Type* StringType;
extern const Type* BooleanType;