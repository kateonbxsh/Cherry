#include "type.h"
#include <data.h>
#include "scope.h"
#include "statements/expression/Expression.h"

namespace {

bool literalValueEquals(const Value& a, const Value& b) {
    if (a.type != b.type) return false;
    if (a.type == nullptr) return true;
    if (a.type == StringType) return std::get<string>(a.value) == std::get<string>(b.value);
    if (a.type == IntegerType) return std::get<integer>(a.value) == std::get<integer>(b.value);
    if (a.type == BooleanType) return std::get<boolean>(a.value) == std::get<boolean>(b.value);
    if (a.type == RealType) return std::get<real>(a.value) == std::get<real>(b.value);
    return false;
}

}

void Type::defineTypes()
{
    IntegerType->setName("integer");
    RealType->setName("float");
    StringType->setName("string");
    BooleanType->setName("boolean");
    FunctionType->setName("function");
    TypeType->setName("type");
    AnonymousType->setName("anon");
    AnyType->setName("any");

    IntegerType->setDefaultValue(Value((integer) 0));
    RealType->setDefaultValue(Value((real) 0.0));
    StringType->setDefaultValue(Value(string("")));
    BooleanType->setDefaultValue(Value((boolean) false));
}

void Type::setName(const string &name)
{
    this->name = name;
}

string Type::getName()
{
    return name;
}

void Type::setDefaultValue(const Value &value)
{
    defaultValue = create_reference<Value>(value);
}

Value Type::getDefaultValue()
{
    if (defaultValue == nullptr) return Value();
    return *defaultValue;
}

bool Type::assignableFrom(const Value& other) {
    if (this->kind == TypeKind::Dynamic) {
        auto resolveDynamicBinding = [&](const reference<Type>& maybeDynamic) -> reference<Type> {
            if (maybeDynamic == nullptr) return nullptr;
            if (maybeDynamic->kind != TypeKind::Dynamic) return maybeDynamic;
            const auto& dynName = maybeDynamic->getName();
            if (dynName.empty()) return maybeDynamic;
            auto it = typeBindings.find(dynName);
            if (it != typeBindings.end() && it->second != nullptr) return it->second;
            return maybeDynamic;
        };

        if (dynamicBaseType != nullptr) {
            auto baseType = resolveDynamicBinding(dynamicBaseType);
            if (baseType == nullptr || !baseType->assignableFrom(other)) return false;
        }

        if (dynamicPredicate != nullptr && !dynamicVariableName.empty()) {
            Scope evalScope;
            evalScope.addVariable(dynamicVariableName, other);
            Value predicate = dynamicPredicate->execute(evalScope);
            if (predicate.thrownException != nullptr) return false;
            if (predicate.type == BooleanType) return std::get<boolean>(predicate.value);
            return predicate.type != nullptr;
        }

        if (!dynamicUnionTypes.empty() || !dynamicUnionLiterals.empty()) {
            for (auto& t : dynamicUnionTypes) {
                auto candidate = resolveDynamicBinding(t);
                if (candidate != nullptr && candidate->assignableFrom(other)) return true;
            }
            for (auto& lit : dynamicUnionLiterals) {
                if (lit != nullptr && literalValueEquals(*lit, other)) return true;
            }
            return false;
        }

        return true;
    }
    if (other.type == nullptr) {
        return false;
    }
    switch(other.type->kind) {
        case TypeKind::Primitive:
            return other.type->getName() == this->getName();
        case TypeKind::Class:
            {
                reference<Type> cursor = other.type;
                while (cursor != nullptr) {
                    if (cursor.get() == this || (!this->getName().empty() && this->getName() == cursor->getName())) {
                        auto a = typeParameters;
                        auto b = cursor->typeParameters;
                        if (a.size() != b.size()) return true;
                        return std::equal(a.begin(), a.end(), b.begin(),
                            [](const TypeParameter& x, const TypeParameter& y) {
                                if (x.value == nullptr || y.value == nullptr) return true;
                                return x.value->assignableFrom(y.value->getDefaultValue());
                            });
                    }
                    cursor = cursor->parent;
                }
                return false;
            }
        case TypeKind::Dynamic:
            return false;
    }
    
    return false;
}
