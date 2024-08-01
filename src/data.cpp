#include "data.h"

Value::Value(reference<Type>& type)
{
    this->type = type;
    initialized = false;
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