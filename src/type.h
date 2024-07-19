
enum PrimitiveType {

    NOT_PRIMITIVE,
    PRIMITIVE_INTEGER,
    PRIMITIVE_FLOAT,
    PRIMITIVE_STRING,
    PRIMITIVE_BOOLEAN

};

class Type {

public:

    Type(): primitive(false), primitiveType(NOT_PRIMITIVE) {};
    explicit Type(PrimitiveType type): primitive(true), primitiveType(type) {};

    bool primitive;
    PrimitiveType primitiveType;

};