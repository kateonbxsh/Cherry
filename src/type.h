
enum PrimitiveType {

    NOT_PRIMITIVE,
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN

};

class Type {

public:

    Type(): primitive(false), primitiveType(PrimitiveType::NOT_PRIMITIVE) {};
    Type(PrimitiveType type): primitive(true), primitiveType(type) {};

    bool primitive;
    PrimitiveType primitiveType;

};