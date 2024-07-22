
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

const Type* NullType = nullptr;
const Type* IntegerType = new Type(PrimitiveType::PRIMITIVE_INTEGER);
const Type* FloatType = new Type(PrimitiveType::PRIMITIVE_FLOAT);
const Type* StringType = new Type(PrimitiveType::PRIMITIVE_STRING);
const Type* BooleanType = new Type(PrimitiveType::PRIMITIVE_BOOLEAN);