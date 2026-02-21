#include "types/type.h"

reference<Type> IntegerType = create_reference<Type>(TypeKind::Primitive);
reference<Type> RealType = create_reference<Type>(TypeKind::Primitive);
reference<Type> StringType = create_reference<Type>(TypeKind::Primitive);
reference<Type> BooleanType = create_reference<Type>(TypeKind::Primitive);
reference<Type> FunctionType = create_reference<Type>(TypeKind::Primitive);
reference<Type> AnyType = create_reference<Type>(TypeKind::Dynamic);
reference<Type> AnonymousType = create_reference<Type>(TypeKind::Dynamic);
reference<Type> TypeType = create_reference<Type>(TypeKind::Primitive);
