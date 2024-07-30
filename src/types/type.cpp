#include "type.h"

void Type::defineTypes()
{
    IntegerType->setName("integer");
    FloatType->setName("float");
    StringType->setName("string");
    BooleanType->setName("boolean");

}

void Type::setName(const std::string &name)
{
    this->name = name;
}

std::string Type::getName()
{
    return name;
}