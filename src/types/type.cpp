#include "type.h"
#include "data.h"

void Type::defineTypes()
{
    IntegerType->setName("integer");
    RealType->setName("float");
    StringType->setName("string");
    BooleanType->setName("boolean");
    FunctionType->setName("function");
    TypeType->setName("type");
    AnonymousType->setName("anon");
    AnonymousType->setName("any");
}

void Type::setName(const string &name)
{
    this->name = name;
}

string Type::getName()
{
    return name;
}

bool Type::assignableFrom(const Value& other) {
    switch(other.type->kind) {
        case TypeKind::Primitive:
            return other.type->getName() == this->getName();
        case TypeKind::Class:
            if (this->getName() == other.type->getName()) {
                auto a = typeParameters;
                auto b = other.type->typeParameters;
                return a.size() == b.size() &&
                    std::equal(a.begin(), a.end(), b.begin(),
                        [](const TypeParameter& x, const TypeParameter& y) {
                            return x.value->assignableFrom(y.value->getDefaultValue());
                        });
            }
            if (parent) {
                return parent->assignableFrom(other);
            }
        case TypeKind::Dynamic:
            return false;
    }
    
    return false;
}