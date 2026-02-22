#include "runtime_builtins.h"
#include <iostream>
#include <sstream>
#include "expressions.h"
#include "scope.h"
#include "types/function.h"
#include "runtime_exception.h"

namespace {

struct NativeArray {
    std::vector<Value> items;
};

struct NativeMap {
    std::unordered_map<std::string, Value> entries;
};

reference<Type> arrayType = nullptr;
reference<Type> mapType = nullptr;
reference<Type> standardType = nullptr;
reference<Type> exceptionType = nullptr;
reference<Type> runtimeExceptionType = nullptr;
reference<Type> valueExceptionType = nullptr;
reference<Type> typeExceptionType = nullptr;
reference<Type> nameExceptionType = nullptr;
reference<Type> argumentExceptionType = nullptr;
reference<Type> operationExceptionType = nullptr;
reference<Type> divisionByZeroExceptionType = nullptr;
reference<Type> indexExceptionType = nullptr;
bool initialized = false;

Function makeInternal(
    const std::vector<FunctionParameter>& params,
    const std::function<Value(Scope&, const std::vector<Value>&, const reference<Value>&)>& handler
) {
    Function fn;
    fn.parameters = params;
    fn.kind = FunctionKind::Internal;
    fn.internalHandler = handler;
    return fn;
}

Value runtimeError(const std::string& message) {
    return makeThrown("RuntimeException", message);
}

reference<Type> getBoundType(const reference<Type>& classType, const std::string& name) {
    reference<Type> cursor = classType;
    while (cursor != nullptr) {
        if (cursor->typeBindings.contains(name)) {
            return cursor->typeBindings.at(name);
        }
        cursor = cursor->parent;
    }
    return nullptr;
}

bool valuesEqual(const Value& a, const Value& b) {
    if (a.type == nullptr || b.type == nullptr) return a.type == b.type;
    if (a.type == StringType && b.type == StringType) return get<string>(a.value) == get<string>(b.value);
    if (a.type == IntegerType && b.type == IntegerType) return get<integer>(a.value) == get<integer>(b.value);
    if (a.type == RealType && b.type == RealType) return get<real>(a.value) == get<real>(b.value);
    if (a.type == BooleanType && b.type == BooleanType) return get<boolean>(a.value) == get<boolean>(b.value);
    Value eq = performBinaryOperator(a, b, COMPARATIVE_EQUALS);
    if (eq.thrownException == nullptr) return isTruthy(eq);
    return false;
}

std::string resolveFormat(const std::string& format, const std::vector<Value>& args) {
    std::string out;
    out.reserve(format.size() + args.size() * 4);
    size_t autoIndex = 0;

    for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] != '{') {
            out.push_back(format[i]);
            continue;
        }
        size_t close = format.find('}', i + 1);
        if (close == std::string::npos) {
            out.push_back('{');
            continue;
        }

        std::string token = format.substr(i + 1, close - i - 1);
        size_t index = 0;
        if (token.empty()) {
            index = autoIndex++;
        } else {
            try {
                index = (size_t)std::stoul(token);
            } catch (...) {
                out.append(format.substr(i, close - i + 1));
                i = close;
                continue;
            }
        }

        if (index < args.size()) {
            out += stringify(args[index]);
        } else {
            out.append("{}");
        }
        i = close;
    }

    return out;
}

}

reference<Type> getArrayTypeBuiltin() {
    return arrayType;
}

reference<Type> getExceptionTypeBuiltin() {
    return exceptionType;
}

reference<Type> getExceptionTypeByName(const std::string& name) {
    if (name == "Exception") return exceptionType;
    if (name == "RuntimeException") return runtimeExceptionType;
    if (name == "ValueException") return valueExceptionType;
    if (name == "TypeException") return typeExceptionType;
    if (name == "NameException") return nameExceptionType;
    if (name == "ArgumentException") return argumentExceptionType;
    if (name == "OperationException") return operationExceptionType;
    if (name == "DivisionByZeroException") return divisionByZeroExceptionType;
    if (name == "IndexException") return indexExceptionType;
    return nullptr;
}

void initializeBuiltinInstance(ClassInstance& instance) {
    if (instance.classType == nullptr) return;
    const auto& name = instance.classType->getName();
    if (name == "Array") {
        if (instance.nativeData == nullptr) {
            instance.nativeData = std::static_pointer_cast<void>(create_reference<NativeArray>());
        }
    } else if (name == "Map") {
        if (instance.nativeData == nullptr) {
            instance.nativeData = std::static_pointer_cast<void>(create_reference<NativeMap>());
        }
    }
}

bool tryGetArrayItems(Value& value, std::vector<Value>*& out) {
    out = nullptr;
    if (value.type == nullptr || value.type->kind != TypeKind::Class) return false;
    auto instance = get<ClassInstance>(value.value);
    if (instance.classType == nullptr || instance.classType->getName() != "Array") return false;
    initializeBuiltinInstance(instance);
    auto data = std::static_pointer_cast<NativeArray>(instance.nativeData);
    out = &data->items;
    value.value = instance;
    return true;
}

bool tryGetMapEntries(Value& value, std::unordered_map<std::string, Value>*& out) {
    out = nullptr;
    if (value.type == nullptr || value.type->kind != TypeKind::Class) return false;
    auto instance = get<ClassInstance>(value.value);
    if (instance.classType == nullptr || instance.classType->getName() != "Map") return false;
    initializeBuiltinInstance(instance);
    auto data = std::static_pointer_cast<NativeMap>(instance.nativeData);
    out = &data->entries;
    value.value = instance;
    return true;
}

Value makeArrayFromValues(const std::vector<Value>& values) {
    if (arrayType == nullptr) return runtimeError("Array type is not initialized");
    ClassInstance instance;
    instance.classType = arrayType;
    initializeBuiltinInstance(instance);
    auto data = std::static_pointer_cast<NativeArray>(instance.nativeData);
    data->items = values;
    Value ret;
    ret.type = arrayType;
    ret.value = instance;
    return ret;
}

void registerBuiltinRuntime(Scope& scope) {
    if (!initialized) {
        initialized = true;

    arrayType = create_reference<Type>(TypeKind::Class);
    arrayType->setName("Array");
    arrayType->typeParameters.push_back({"T", nullptr, true, AnyType});
    arrayType->typeBindings["T"] = nullptr;

    {
        Method pushMethod;
        pushMethod.flags = MemberFlags::Public;
        FunctionParameter p;
        p.name = "value";
        p.type = AnyType;
        pushMethod.overloads.push_back(makeInternal({p}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.push missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.push receiver is not an Array");
            auto instance = get<ClassInstance>(thisValue.value);
            auto elementType = getBoundType(instance.classType, "T");
            if (elementType != nullptr && args[0].type != nullptr && !elementType->assignableFrom(args[0])) {
                return runtimeError(
                    "Array.push expected value of type " + elementType->getName() + ", got " + args[0].type->getName()
                );
            }
            items->push_back(args[0]);
            return Value((integer)items->size());
        }));
        arrayType->methods["push"] = std::move(pushMethod);
    }
    {
        Method sizeMethod;
        sizeMethod.flags = MemberFlags::Public;
        sizeMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.size missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.size receiver is not an Array");
            return Value((integer)items->size());
        }));
        arrayType->methods["size"] = std::move(sizeMethod);
    }
    {
        Method getMethod;
        getMethod.flags = MemberFlags::Public;
        FunctionParameter indexParam{"index", IntegerType, false};
        getMethod.overloads.push_back(makeInternal({indexParam}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.get missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.get receiver is not an Array");
            integer idx = get<integer>(args[0].value);
            if (idx < 0 || (size_t)idx >= items->size()) return runtimeError("Array index out of range");
            return (*items)[(size_t)idx];
        }));
        arrayType->methods["get"] = std::move(getMethod);
    }
    {
        Method setMethod;
        setMethod.flags = MemberFlags::Public;
        FunctionParameter indexParam{"index", IntegerType, false};
        FunctionParameter valueParam{"value", AnyType, false};
        setMethod.overloads.push_back(makeInternal({indexParam, valueParam}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.set missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.set receiver is not an Array");
            integer idx = get<integer>(args[0].value);
            if (idx < 0 || (size_t)idx >= items->size()) return runtimeError("Array index out of range");
            auto instance = get<ClassInstance>(thisValue.value);
            auto elementType = getBoundType(instance.classType, "T");
            if (elementType != nullptr && args[1].type != nullptr && !elementType->assignableFrom(args[1])) {
                return runtimeError(
                    "Array.set expected value of type " + elementType->getName() + ", got " + args[1].type->getName()
                );
            }
            (*items)[(size_t)idx] = args[1];
            return args[1];
        }));
        arrayType->methods["set"] = std::move(setMethod);
    }
    {
        Method popMethod;
        popMethod.flags = MemberFlags::Public;
        popMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.pop missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.pop receiver is not an Array");
            if (items->empty()) return runtimeError("Array.pop on empty array");
            Value ret = items->back();
            items->pop_back();
            return ret;
        }));
        arrayType->methods["pop"] = std::move(popMethod);
    }
    {
        Method clearMethod;
        clearMethod.flags = MemberFlags::Public;
        clearMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.clear missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.clear receiver is not an Array");
            items->clear();
            return NullValue;
        }));
        arrayType->methods["clear"] = std::move(clearMethod);
    }
    {
        Method emptyMethod;
        emptyMethod.flags = MemberFlags::Public;
        emptyMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.empty missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.empty receiver is not an Array");
            return Value((boolean)items->empty());
        }));
        arrayType->methods["empty"] = std::move(emptyMethod);
    }
    {
        Method containsMethod;
        containsMethod.flags = MemberFlags::Public;
        FunctionParameter p{"value", AnyType, false};
        containsMethod.overloads.push_back(makeInternal({p}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Array.contains missing receiver");
            Value thisValue = *self;
            std::vector<Value>* items = nullptr;
            if (!tryGetArrayItems(thisValue, items)) return runtimeError("Array.contains receiver is not an Array");
            for (const auto& v : *items) {
                if (valuesEqual(v, args[0])) return Value((boolean)true);
            }
            return Value((boolean)false);
        }));
        arrayType->methods["contains"] = std::move(containsMethod);
    }

    mapType = create_reference<Type>(TypeKind::Class);
    mapType->setName("Map");
    mapType->typeParameters.push_back({"U", nullptr, true, StringType});
    mapType->typeParameters.push_back({"V", nullptr, true, AnyType});
    mapType->typeBindings["U"] = nullptr;
    mapType->typeBindings["V"] = nullptr;

    {
        Method setMethod;
        setMethod.flags = MemberFlags::Public;
        FunctionParameter k{"key", StringType, false};
        FunctionParameter v{"value", AnyType, false};
        setMethod.overloads.push_back(makeInternal({k, v}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.set missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.set receiver is not a Map");
            auto instance = get<ClassInstance>(thisValue.value);
            auto keyType = getBoundType(instance.classType, "U");
            auto valueType = getBoundType(instance.classType, "V");
            if (keyType != nullptr && args[0].type != nullptr && !keyType->assignableFrom(args[0])) {
                return runtimeError(
                    "Map.set expected key of type " + keyType->getName() + ", got " + args[0].type->getName()
                );
            }
            if (valueType != nullptr && args[1].type != nullptr && !valueType->assignableFrom(args[1])) {
                return runtimeError(
                    "Map.set expected value of type " + valueType->getName() + ", got " + args[1].type->getName()
                );
            }
            if (args[0].type != StringType) {
                return runtimeError("Map currently supports only string keys at runtime");
            }
            entries->insert_or_assign(get<string>(args[0].value), args[1]);
            return args[1];
        }));
        mapType->methods["set"] = std::move(setMethod);
    }
    {
        Method getMethod;
        getMethod.flags = MemberFlags::Public;
        FunctionParameter k{"key", StringType, false};
        getMethod.overloads.push_back(makeInternal({k}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.get missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.get receiver is not a Map");
            auto it = entries->find(get<string>(args[0].value));
            if (it == entries->end()) return NullValue;
            return it->second;
        }));
        mapType->methods["get"] = std::move(getMethod);
    }
    {
        Method hasMethod;
        hasMethod.flags = MemberFlags::Public;
        FunctionParameter k{"key", StringType, false};
        hasMethod.overloads.push_back(makeInternal({k}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.has missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.has receiver is not a Map");
            return Value((boolean)(entries->contains(get<string>(args[0].value))));
        }));
        mapType->methods["has"] = std::move(hasMethod);
    }
    {
        Method removeMethod;
        removeMethod.flags = MemberFlags::Public;
        FunctionParameter k{"key", StringType, false};
        removeMethod.overloads.push_back(makeInternal({k}, [](Scope&, const std::vector<Value>& args, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.remove missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.remove receiver is not a Map");
            return Value((boolean)(entries->erase(get<string>(args[0].value)) > 0));
        }));
        mapType->methods["remove"] = std::move(removeMethod);
    }
    {
        Method sizeMethod;
        sizeMethod.flags = MemberFlags::Public;
        sizeMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.size missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.size receiver is not a Map");
            return Value((integer)entries->size());
        }));
        mapType->methods["size"] = std::move(sizeMethod);
    }
    {
        Method clearMethod;
        clearMethod.flags = MemberFlags::Public;
        clearMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.clear missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.clear receiver is not a Map");
            entries->clear();
            return NullValue;
        }));
        mapType->methods["clear"] = std::move(clearMethod);
    }
    {
        Method emptyMethod;
        emptyMethod.flags = MemberFlags::Public;
        emptyMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.empty missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.empty receiver is not a Map");
            return Value((boolean)entries->empty());
        }));
        mapType->methods["empty"] = std::move(emptyMethod);
    }
    {
        Method keysMethod;
        keysMethod.flags = MemberFlags::Public;
        keysMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.keys missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.keys receiver is not a Map");
            std::vector<Value> keys;
            keys.reserve(entries->size());
            for (const auto& [k, _] : *entries) keys.push_back(Value(k));
            return makeArrayFromValues(keys);
        }));
        mapType->methods["keys"] = std::move(keysMethod);
    }
    {
        Method valuesMethod;
        valuesMethod.flags = MemberFlags::Public;
        valuesMethod.overloads.push_back(makeInternal({}, [](Scope&, const std::vector<Value>&, const reference<Value>& self) -> Value {
            if (self == nullptr) return runtimeError("Map.values missing receiver");
            Value thisValue = *self;
            std::unordered_map<std::string, Value>* entries = nullptr;
            if (!tryGetMapEntries(thisValue, entries)) return runtimeError("Map.values receiver is not a Map");
            std::vector<Value> values;
            values.reserve(entries->size());
            for (const auto& [_, v] : *entries) values.push_back(v);
            return makeArrayFromValues(values);
        }));
        mapType->methods["values"] = std::move(valuesMethod);
    }

    standardType = create_reference<Type>(TypeKind::Class);
    standardType->setName("Standard");

    auto formatBuiltin = []() {
        std::vector<FunctionParameter> params;
        params.push_back({"format", StringType, false});
        params.push_back({"args", AnyType, true});
        return makeInternal(params, [](Scope&, const std::vector<Value>& args, const reference<Value>&) -> Value {
            std::vector<Value> unpacked;
            if (args.size() > 1) {
                Value packed = args[1];
                std::vector<Value>* items = nullptr;
                if (!tryGetArrayItems(packed, items)) return runtimeError("format variadic arguments must be an Array");
                unpacked = *items;
            }
            std::string format = get<string>(args[0].value);
            std::string text = resolveFormat(format, unpacked);
            return Value(text);
        });
    };

    {
        Method formatMethod;
        formatMethod.flags = MemberFlags::Public | MemberFlags::Static;
        formatMethod.overloads.push_back(formatBuiltin());
        standardType->staticMethods["format"] = std::move(formatMethod);
    }
    {
        Method printMethod;
        printMethod.flags = MemberFlags::Public | MemberFlags::Static;
        printMethod.overloads.push_back(makeInternal(
            {{"format", StringType, false}, {"args", AnyType, true}},
            [formatBuiltin](Scope& scope, const std::vector<Value>& args, const reference<Value>& self) -> Value {
                Function formatter = formatBuiltin();
                Value rendered = formatter.internalHandler(scope, args, self);
                if (rendered.thrownException != nullptr) return rendered;
                std::cout << get<string>(rendered.value);
                return NullValue;
            }
        ));
        standardType->staticMethods["print"] = std::move(printMethod);
    }
    {
        Method printlnMethod;
        printlnMethod.flags = MemberFlags::Public | MemberFlags::Static;
        printlnMethod.overloads.push_back(makeInternal(
            {{"format", StringType, false}, {"args", AnyType, true}},
            [formatBuiltin](Scope& scope, const std::vector<Value>& args, const reference<Value>& self) -> Value {
                Function formatter = formatBuiltin();
                Value rendered = formatter.internalHandler(scope, args, self);
                if (rendered.thrownException != nullptr) return rendered;
                std::cout << get<string>(rendered.value) << std::endl;
                return NullValue;
            }
        ));
        standardType->staticMethods["println"] = std::move(printlnMethod);
    }

    exceptionType = create_reference<Type>(TypeKind::Class);
    exceptionType->setName("Exception");
    runtimeExceptionType = create_reference<Type>(TypeKind::Class);
    runtimeExceptionType->setName("RuntimeException");
    valueExceptionType = create_reference<Type>(TypeKind::Class);
    valueExceptionType->setName("ValueException");
    typeExceptionType = create_reference<Type>(TypeKind::Class);
    typeExceptionType->setName("TypeException");
    nameExceptionType = create_reference<Type>(TypeKind::Class);
    nameExceptionType->setName("NameException");
    argumentExceptionType = create_reference<Type>(TypeKind::Class);
    argumentExceptionType->setName("ArgumentException");
    operationExceptionType = create_reference<Type>(TypeKind::Class);
    operationExceptionType->setName("OperationException");
    divisionByZeroExceptionType = create_reference<Type>(TypeKind::Class);
    divisionByZeroExceptionType->setName("DivisionByZeroException");
    indexExceptionType = create_reference<Type>(TypeKind::Class);
    indexExceptionType->setName("IndexException");

    auto makeExceptionField = [](const std::string& name) -> Field {
        Field f;
        f.name = name;
        f.type = nullptr;
        f.flags = MemberFlags::Public;
        f.hasDefaultValue = false;
        f.value = nullptr;
        return f;
    };
    exceptionType->fields.push_back(makeExceptionField("message"));
    exceptionType->fields.push_back(makeExceptionField("stacktrace"));
    exceptionType->fields.push_back(makeExceptionField("line"));
    exceptionType->fields.push_back(makeExceptionField("col"));
    exceptionType->fields.push_back(makeExceptionField("file"));

    runtimeExceptionType->parent = exceptionType;
    valueExceptionType->parent = runtimeExceptionType;
    typeExceptionType->parent = runtimeExceptionType;
    nameExceptionType->parent = runtimeExceptionType;
    argumentExceptionType->parent = runtimeExceptionType;
    operationExceptionType->parent = runtimeExceptionType;
    divisionByZeroExceptionType->parent = operationExceptionType;
    indexExceptionType->parent = runtimeExceptionType;

    }

    scope.addVariable("Array", Value(arrayType));
    scope.addVariable("Map", Value(mapType));
    scope.addVariable("Standard", Value(standardType));

    scope.addVariable("Exception", Value(exceptionType));
    scope.addVariable("RuntimeException", Value(runtimeExceptionType));
    scope.addVariable("ValueException", Value(valueExceptionType));
    scope.addVariable("TypeException", Value(typeExceptionType));
    scope.addVariable("NameException", Value(nameExceptionType));
    scope.addVariable("ArgumentException", Value(argumentExceptionType));
    scope.addVariable("OperationException", Value(operationExceptionType));
    scope.addVariable("DivisionByZeroException", Value(divisionByZeroExceptionType));
    scope.addVariable("IndexException", Value(indexExceptionType));
}
