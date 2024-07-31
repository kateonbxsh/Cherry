#include "type.h"

void Type::defineTypes()
{
    IntegerType->setName("integer");
    RealType->setName("float");
    StringType->setName("string");
    BooleanType->setName("boolean");

}

void Type::setName(const string &name)
{
    this->name = name;
}

string Type::getName()
{
    return name;
}