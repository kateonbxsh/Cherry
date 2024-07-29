
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

Type* IntegerType = new Type(PrimitiveType::PRIMITIVE_INTEGER);
Type* FloatType = new Type(PrimitiveType::PRIMITIVE_FLOAT);
Type* StringType = new Type(PrimitiveType::PRIMITIVE_STRING);
Type* BooleanType = new Type(PrimitiveType::PRIMITIVE_BOOLEAN);