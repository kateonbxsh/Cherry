#include "types/type.h"

reference<Type> IntegerType = create_reference<Type>(PrimitiveType::PRIMITIVE_INTEGER);
reference<Type> RealType = create_reference<Type>(PrimitiveType::PRIMITIVE_REAL);
reference<Type> StringType = create_reference<Type>(PrimitiveType::PRIMITIVE_STRING);
reference<Type> BooleanType = create_reference<Type>(PrimitiveType::PRIMITIVE_BOOLEAN);