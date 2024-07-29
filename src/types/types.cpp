#include "types/type.h"

const Type* IntegerType = new Type(PrimitiveType::PRIMITIVE_INTEGER);
const Type* FloatType = new Type(PrimitiveType::PRIMITIVE_FLOAT);
const Type* StringType = new Type(PrimitiveType::PRIMITIVE_STRING);
const Type* BooleanType = new Type(PrimitiveType::PRIMITIVE_BOOLEAN);