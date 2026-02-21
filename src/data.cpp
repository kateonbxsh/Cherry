#include "data.h"
#include "types/function.h"

Value Value::Uninitialized(reference<Type>& type) {
    if (type && type->defaultValue != nullptr) {
        return *type->defaultValue;
    }

    Value value;
    value.type = type;
    value.value = nullptr;
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
