#include "data.h"
#include "types/function.h"

Value Value::Uninitialized(reference<Type>& type) {
    Value value;
    value.type = type;
    if (type == IntegerType) {
        value.value = 0;
    } else if (type == RealType) {
        value.value = (real) 0;
    } else if (type == BooleanType) {
        value.value = (boolean) false;
    } else if (type == StringType) {
        value.value = (string) "";
    } else if (type == TypeType) {
        value.value = AnonymousType;
    }
    return value;
}

Value::Value(reference<Type>& type)
{
    this->value = type;
    this->type = TypeType;
}

Value::Value(boolean a)
{
    type=BooleanType;
    value=a;
}

Value::Value(integer a)
{
    type=IntegerType;
    value=a;
}

Value::Value(real a)
{
    type=RealType;
    value=a;
}

Value::Value(string a)
{
    type=StringType;
    value=a;
}

Value::Value(Function function)
{
    type=FunctionType;
    value=function;
}