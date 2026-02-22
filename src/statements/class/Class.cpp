#include "Class.hpp"
#include "runtime_exception.h"

namespace {

Value makeClassRegistrationError(const string& message) {
    return makeThrown("TypeException", message);
}

bool isOverloadableBinaryOperator(TokenKind kind) {
    return kind > BEGIN_OF_BINARY_OPERATORS && kind < END_OF_BINARY_OPERATORS && kind != IS;
}

bool acceptsParamType(const reference<Type>& targetType, const reference<Type>& sourceType) {
    if (targetType == nullptr || sourceType == nullptr) return false;
    auto sourceCopy = sourceType;
    Value probe = Value::Uninitialized(sourceCopy);
    probe.type = sourceType;
    return targetType->assignableFrom(probe);
}

reference<Type> resolveParameterType(const reference<Type>& ownerType, const reference<Type>& parameterType) {
    if (ownerType == nullptr || parameterType == nullptr) return parameterType;
    if (parameterType->kind != TypeKind::Dynamic) return parameterType;
    auto paramName = parameterType->getName();
    reference<Type> cursor = ownerType;
    while (cursor != nullptr) {
        if (cursor->typeBindings.contains(paramName)) {
            auto bound = cursor->typeBindings.at(paramName);
            if (bound != nullptr) return bound;
            break;
        }
        cursor = cursor->parent;
    }
    return parameterType;
}

bool overloadDirectionallyCovers(const reference<Type>& ownerType, const Function& a, const Function& b) {
    const bool aVariadic = !a.parameters.empty() && a.parameters.back().variadic;
    const bool bVariadic = !b.parameters.empty() && b.parameters.back().variadic;
    if (aVariadic != bVariadic) return false;

    size_t aFixed = a.parameters.size() - (aVariadic ? 1 : 0);
    size_t bFixed = b.parameters.size() - (bVariadic ? 1 : 0);
    if (aFixed != bFixed) return false;

    for (size_t i = 0; i < aFixed; ++i) {
        auto aType = resolveParameterType(ownerType, a.parameters[i].type);
        auto bType = resolveParameterType(ownerType, b.parameters[i].type);
        if (aType != nullptr && aType == a.parameters[i].type && aType->kind == TypeKind::Dynamic) return false;
        if (bType != nullptr && bType == b.parameters[i].type && bType->kind == TypeKind::Dynamic) return false;
        if (!acceptsParamType(aType, bType)) return false;
    }

    if (aVariadic) {
        auto aType = resolveParameterType(ownerType, a.parameters.back().type);
        auto bType = resolveParameterType(ownerType, b.parameters.back().type);
        if (aType != nullptr && aType == a.parameters.back().type && aType->kind == TypeKind::Dynamic) return false;
        if (bType != nullptr && bType == b.parameters.back().type && bType->kind == TypeKind::Dynamic) return false;
        if (!acceptsParamType(aType, bType)) return false;
    }

    return true;
}

Value ensureNoCompatibleOverloads(const reference<Type>& ownerType, const Method& method, const string& owner, const string& methodName) {
    for (size_t i = 0; i < method.overloads.size(); ++i) {
        for (size_t j = i + 1; j < method.overloads.size(); ++j) {
            const auto& a = method.overloads[i];
            const auto& b = method.overloads[j];
            if (overloadDirectionallyCovers(ownerType, a, b) || overloadDirectionallyCovers(ownerType, b, a)) {
                return makeClassRegistrationError(
                    "ambiguous overloads for " + owner + "." + methodName + ": signatures are directionally compatible"
                );
            }
        }
    }
    return NullValue;
}

}

uref<ClassDeclaration> ClassDeclaration::parse(Lexer& lexer) {

    lexer.savePosition();
    auto klass = create_unique<ClassDeclaration>();
    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering ClassDeclaration::parse\n";

    // class keyword
    if (!lexer.expectToken(CLASS)) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "No class keyword found\n";
        klass->valid = false;
        lexer.rollPosition();
        return klass;
    }

    // class name
    Token name = lexer.nextToken();
    if (name.kind != IDENTIFIER) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected class name\n";
        klass->valid = false;
        klass->expected = {"class name"};
        klass->lastToken = name;
        lexer.rollPosition();
        return klass;
    }

    klass->name = name.value;
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsing class: " << klass->name << "\n";

    if (lexer.expectToken(EXTENDS)) {
        TypeSyntaxExpression baseTypeExpr;
        if (!parseTypeSyntaxExpression(lexer, baseTypeExpr, *klass)) {
            klass->valid = false;
            lexer.rollPosition();
            return klass;
        }
        klass->hasBaseType = true;
        klass->baseType = baseTypeExpr;
    }

    /* =========================
       Type parameters
       ========================= */
    if (lexer.expectToken(SMALLER_THAN)) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing class type parameters\n";
        bool seenDefault = false;

        while (true) {

            Token typeName = lexer.nextToken();
            if (typeName.kind != IDENTIFIER) {
                if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected type parameter name\n";
                klass->valid = false;
                klass->expected = {"type parameter"};
                klass->lastToken = typeName;
                lexer.rollPosition();
                return klass;
            }

            ClassDeclarationTypeParameter param;
            param.name = typeName;
            param.withDefaultValue = false;
            param.withConstraint = false;

            if (lexer.expectToken(EXTENDS)) {
                TypeSyntaxExpression constraintExpr;
                if (!parseTypeSyntaxExpression(lexer, constraintExpr, *klass)) {
                    klass->valid = false;
                    lexer.rollPosition();
                    return klass;
                }
                param.withConstraint = true;
                param.constraintType = constraintExpr;
            }

            if (lexer.expectToken(EQUALS)) {
                if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing type parameter default value\n";
                TypeSyntaxExpression defExpr;
                if (!parseTypeSyntaxExpression(lexer, defExpr, *klass)) {
                    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid type parameter default type\n";
                    klass->valid = false;
                    lexer.rollPosition();
                    return klass;
                }

                param.withDefaultValue = true;
                param.defaultValue = defExpr;
                seenDefault = true;
            } else if (seenDefault) {
                if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Non-default type parameter after default\n";
                klass->valid = false;
                klass->expected = {"type parameter with default value"};
                klass->lastToken = typeName;
                klass->errorMessage = "type parameters with default values must be at the end of the list";
                lexer.rollPosition();
                return klass;
            }

            klass->typeParameters.push_back(move(param));
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Added type parameter: " << typeName.value << "\n";

            Token sep = lexer.nextToken();
            if (sep.kind == BIGGER_THAN) break;

            if (sep.kind != COMMA) {
                if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected ',' or '>' in type parameter list\n";
                klass->valid = false;
                klass->expected = tokenKindsToString({COMMA, BIGGER_THAN});
                klass->lastToken = sep;
                lexer.rollPosition();
                return klass;
            }
        }
    }

    if (!klass->hasBaseType && lexer.expectToken(EXTENDS)) {
        TypeSyntaxExpression baseTypeExpr;
        if (!parseTypeSyntaxExpression(lexer, baseTypeExpr, *klass)) {
            klass->valid = false;
            lexer.rollPosition();
            return klass;
        }
        klass->hasBaseType = true;
        klass->baseType = baseTypeExpr;
    }

    /* =========================
       Class body
       ========================= */
    if (!lexer.expectToken(LEFT_BRACE)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected '{' to start class body\n";
        klass->valid = false;
        klass->expected = {"{"};
        klass->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return klass;
    }
    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing class body\n";

    while (!lexer.expectToken(RIGHT_BRACE)) {

        lexer.savePosition();

        // Try method
        if (DEBUG) std::cout << DEBUG_PREFIX << "Trying MethodDefinition::parse in class body\n";
        DEBUG_TABS++;
        auto method = MethodDefinition::parse(lexer);
        auto methodError = create_unique<NotAStatement>();
        methodError->valid = false;
        methodError->expected = method->expected;
        methodError->lastToken = method->lastToken;
        methodError->errorMessage = method->errorMessage;
        DEBUG_TABS--;
        if (!method->valid && !method->errorMessage.empty()) {
            klass->valid = false;
            klass->expected = method->expected;
            klass->lastToken = method->lastToken;
            klass->errorMessage = method->errorMessage;
            lexer.rollPosition();
            return klass;
        }
        if (method->valid) {
            method->isConstructor = (method->name.value == klass->name);
            if (method->isConstructor) {
                klass->constructors.push_back(move(method));
                if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed constructor definition\n";
            } else {
                klass->methods[method->name.value].push_back(move(method));
                if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed method definition\n";
            }
            lexer.deletePosition();
            continue;
        }

        lexer.rollPosition();
        lexer.savePosition();

        // Try field
        if (DEBUG) std::cout << DEBUG_PREFIX << "Trying FieldDefinition::parse in class body\n";
        DEBUG_TABS++;
        auto field = FieldDefinition::parse(lexer);
        auto fieldError = create_unique<NotAStatement>();
        fieldError->valid = false;
        fieldError->expected = field.expected;
        fieldError->lastToken = field.lastToken;
        fieldError->errorMessage = field.errorMessage;
        DEBUG_TABS--;
        if (field.valid) {
            klass->fields.push_back(field);
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed field definition: " << field.name.value << "\n";
            lexer.deletePosition();
            continue;
        }

        lexer.rollPosition();

        // Nothing matched
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected field or method in class body\n";
        klass->valid = false;
        std::vector<uref<Statement>> invalids;
        invalids.push_back(move(methodError));
        invalids.push_back(move(fieldError));
        auto furthest = getFurthestInvalidStatement(invalids);
        if (furthest != nullptr) {
            klass->expected = furthest->expected;
            klass->lastToken = furthest->lastToken;
            klass->errorMessage = furthest->errorMessage;
        } else {
            klass->expected = {"field or method"};
            klass->lastToken = lexer.nextToken();
        }
        lexer.rollPosition();
        return klass;
    }

    klass->valid = true;
    if (DEBUG) {
        std::cout << DEBUG_SUCCESS_PREFIX << "Class parsed successfully. Fields: " << klass->fields.size()
                  << ", method names: " << klass->methods.size() << "\n";
    }
    lexer.deletePosition();
    return klass;
}

FieldDefinition FieldDefinition::parse(Lexer& lexer) {

    FieldDefinition fieldDef;
    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering FieldDefinition::parse\n";
    fieldDef.flags = 0;
    fieldDef.withValue = false;

    bool hasAccessModifier = false;
    TokenKind accessKind = NONE;
    while (true) {
        Token token = lexer.peekToken();

        if (token.kind == PUBLIC || token.kind == PRIVATE || token.kind == PROTECTED) {
            if (hasAccessModifier && token.kind != accessKind) {
                fieldDef.valid = false;
                fieldDef.expected = {"single access modifier"};
                fieldDef.lastToken = token;
                fieldDef.errorMessage = "cannot combine multiple access modifiers";
                return fieldDef;
            }
            hasAccessModifier = true;
            accessKind = token.kind;
            if (token.kind == PUBLIC) fieldDef.flags |= MemberFlags::Public;
            else if (token.kind == PRIVATE) fieldDef.flags |= MemberFlags::Private;
            else fieldDef.flags |= MemberFlags::Protected;
            lexer.nextToken();
            continue;
        }

        if (token.kind == STATIC) {
            fieldDef.flags |= MemberFlags::Static;
            lexer.nextToken();
            continue;
        }

        if (token.kind == IDENTIFIER && token.value == "readonly") {
            fieldDef.flags |= MemberFlags::Readonly;
            lexer.nextToken();
            continue;
        }

        break;
    }

    if (!hasAccessModifier) {
        fieldDef.flags |= MemberFlags::Private;
    }

    auto typeExpr = Expression::parse(lexer);
    if (!typeExpr->valid) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid field type expression\n";
        fieldDef.invalidFrom(*typeExpr);
        return fieldDef;
    }
    fieldDef.type = move(typeExpr);
    
    if (!lexer.expectToken(IDENTIFIER)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected field name\n";
        fieldDef.invalidExpected({IDENTIFIER}, lexer);
        return fieldDef;
    }

    fieldDef.name = lexer.currentToken();

    if (lexer.expectToken(EQUALS)) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing field default value\n";
        auto expr = Expression::parse(lexer);
        if (!expr->valid) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid field default value expression\n";
            fieldDef.invalidFrom(*expr);
            return fieldDef;
        }
        fieldDef.withValue = true;
        fieldDef.value = move(expr);
    }

    if (!lexer.expectToken(SEMICOLON)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected ';' after field definition\n";
        fieldDef.invalidExpected({SEMICOLON}, lexer);
        return fieldDef;
    }

    fieldDef.valid = true;
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Field parsed successfully: " << fieldDef.name.value << "\n";
    return fieldDef;

}

Value ClassDeclaration::execute(Scope& scope) {
    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering ClassDeclaration::execute for class: " << name << "\n";

    // Create class type
    auto classType = create_reference<Type>(TypeKind::Class);
    classType->setName(name);
    scope.addVariable(name, Value(classType));

    Scope classScope = scope.createChild();
    classScope.addVariable(name, Value(classType));

    if (hasBaseType) {
        Value parentTypeValue = resolveTypeSyntaxExpression(classScope, baseType, false, "");
        if (parentTypeValue.thrownException != nullptr) return parentTypeValue;
        if (parentTypeValue.type != TypeType) {
            return makeClassRegistrationError("extended type is not a type");
        }
        auto parentType = get<reference<Type>>(parentTypeValue.value);
        if (parentType == nullptr || parentType->kind != TypeKind::Class) {
            return makeClassRegistrationError("class can only extend class type");
        }
        classType->parent = parentType;
    }

    /* =========================
       Type parameters
       ========================= */
    for (auto& param : typeParameters) {
        auto paramName = param.name.value;
        if (DEBUG) std::cout << DEBUG_PREFIX << "Resolving type parameter: " << paramName << "\n";

        TypeParameter tp;
        tp.name = paramName;
        tp.hasDefault = param.withDefaultValue;
        tp.hasConstraint = param.withConstraint;

        if (param.withDefaultValue) {
            Value def = resolveTypeSyntaxExpression(classScope, param.defaultValue, false, "");
            if (def.thrownException != nullptr) return def;
            if (def.type != TypeType) return Value{};
            tp.defaultValue = get<reference<Type>>(def.value);
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Resolved type parameter default: " << paramName << "\n";
        }

        if (param.withConstraint) {
            Value constraint = resolveTypeSyntaxExpression(classScope, param.constraintType, false, "");
            if (constraint.thrownException != nullptr) return constraint;
            if (constraint.type != TypeType) {
                return makeClassRegistrationError("type parameter constraint is not a type");
            }
            tp.constraint = get<reference<Type>>(constraint.value);
        }

        classType->typeParameters.push_back(tp);
        classType->typeBindings[paramName] = nullptr;

        if (tp.hasDefault && tp.defaultValue != nullptr) {
            classScope.addVariable(paramName, Value(tp.defaultValue));
        } else {
            auto placeholderType = create_reference<Type>(TypeKind::Dynamic);
            placeholderType->setName(paramName);
            classScope.addVariable(paramName, Value(placeholderType));
        }
    }


    for (auto& field : fields) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Registering field: " << field.name.value << "\n";

        Field f;
        f.name = field.name.value;
        f.type = move(field.type);
        f.flags = field.flags;
        f.hasDefaultValue = false;

        if (field.withValue) {
            f.hasDefaultValue = true;
            f.value = move(field.value);
        }

        if ((f.flags & MemberFlags::Static) != 0) {
            classType->staticFields.push_back(f);

            Value staticValue;
            if (f.hasDefaultValue) {
                staticValue = f.value->execute(classScope);
            } else {
                Value fieldTypeValue = f.type->execute(classScope);
                if (fieldTypeValue.thrownException != nullptr) return fieldTypeValue;
                if (fieldTypeValue.type != TypeType) {
                    return makeClassRegistrationError("static field type is not a type");
                }
                auto staticFieldType = get<reference<Type>>(fieldTypeValue.value);
                staticValue = Value::Uninitialized(staticFieldType);
            }

            if (staticValue.thrownException != nullptr) return staticValue;
            classType->staticFieldValues[f.name] = create_reference<Value>(staticValue);
        } else {
            classType->fields.push_back(f);
        }
    }

    Scope methodScope = classScope.createChild();
    for (const auto& tp : classType->typeParameters) {
        auto placeholderType = create_reference<Type>(TypeKind::Dynamic);
        placeholderType->setName(tp.name);
        methodScope.addVariable(tp.name, Value(placeholderType));
    }

    /* =========================
       Constructor
       ========================= */
    if (!constructors.empty()) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Registering constructors\n";

        Method constructor;
        constructor.flags = constructors.front()->flags;

        for (auto& constructorDef : constructors) {
            Value constructorValue = constructorDef->execute(methodScope);
            if (constructorValue.thrownException != nullptr) return constructorValue;
            Function fn = get<Function>(constructorValue.value);
            fn.debugName = name + "." + constructorDef->name.value;
            constructor.overloads.push_back(fn);
        }

        Value overloadCheck = ensureNoCompatibleOverloads(classType, constructor, name, name);
        if (overloadCheck.thrownException != nullptr) return overloadCheck;

        classType->constructor = move(constructor);
        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Registered constructor overload count: " << classType->constructor.overloads.size() << "\n";
    }

    /* =========================
       Methods
       ========================= */
    for (auto& [methodName, methodDefinitions] : methods) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Registering methods for name: " << methodName << "\n";

        Method method;
        method.flags = methodDefinitions.empty() ? MemberFlags::Private : methodDefinitions.front()->flags;

        for (auto& methodDef : methodDefinitions) {
            Value methodValue = methodDef->execute(methodScope);
            if (methodValue.thrownException != nullptr) return methodValue;
            Function fn = get<Function>(methodValue.value);
            fn.debugName = name + "." + methodName;
            method.overloads.push_back(fn);
        }

        Value overloadCheck = ensureNoCompatibleOverloads(classType, method, name, methodName);
        if (overloadCheck.thrownException != nullptr) return overloadCheck;

        if ((method.flags & MemberFlags::Static) != 0) {
            classType->staticMethods[methodName] = move(method);
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Registered static overload count: " << classType->staticMethods[methodName].overloads.size() << "\n";
        } else {
            classType->methods[methodName] = move(method);
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Registered overload count: " << classType->methods[methodName].overloads.size() << "\n";
        }
    }

    /* =========================
       Register class in scope
       ========================= */
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Class registered in scope: " << name << "\n";

    return Value(classType);
}


uref<MethodDefinition> MethodDefinition::parse(Lexer& lexer) {

    lexer.savePosition();
    auto method = create_unique<MethodDefinition>();
    if (DEBUG) std::cout << DEBUG_PREFIX << "Entering MethodDefinition::parse\n";

    method->flags = 0;
    bool hasAccessModifier = false;
    TokenKind accessKind = NONE;
    while (true) {
        Token token = lexer.peekToken();

        if (token.kind == PUBLIC || token.kind == PRIVATE || token.kind == PROTECTED) {
            if (hasAccessModifier && token.kind != accessKind) {
                method->valid = false;
                method->expected = {"single access modifier"};
                method->lastToken = token;
                method->errorMessage = "cannot combine multiple access modifiers";
                lexer.rollPosition();
                return method;
            }
            hasAccessModifier = true;
            accessKind = token.kind;
            if (token.kind == PUBLIC) method->flags |= MemberFlags::Public;
            else if (token.kind == PRIVATE) method->flags |= MemberFlags::Private;
            else method->flags |= MemberFlags::Protected;
            lexer.nextToken();
            continue;
        }

        if (token.kind == STATIC) {
            method->flags |= MemberFlags::Static;
            lexer.nextToken();
            continue;
        }

        break;
    }

    if (!hasAccessModifier) {
        method->flags |= MemberFlags::Private;
    }

    Token name = lexer.nextToken();
    if (name.kind != IDENTIFIER && name.kind != DISPLAY) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "No method name found\n";
        method->valid = false;
        lexer.rollPosition();
        return method;
    }

    bool isOperatorOverload = false;
    if (name.value == "operator") {
        Token opToken = lexer.nextToken();
        if (!isOverloadableBinaryOperator(opToken.kind)) {
            method->valid = false;
            method->expected = {"binary operator"};
            method->lastToken = opToken;
            method->errorMessage = "operator overload must use a binary operator token";
            lexer.rollPosition();
            return method;
        }
        method->name = Token{IDENTIFIER, "operator" + opToken.value, name.pos, name.line};
        isOperatorOverload = true;
    } else if (name.kind == DISPLAY) {
        method->name = Token{IDENTIFIER, "display", name.pos, name.line};
    } else {
        method->name = name;
    }
    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing method: " << method->name.value << "\n";

    TokenKind listClose = RIGHT_PARENTHESIS;
    bool isIndexer = false;
    bool isDisplayShortcut = method->name.value == "display" && lexer.peekToken().kind == LEFT_BRACE;
    if (isDisplayShortcut) {
        // display { ... } is sugar for display() { ... }
    } else if ((method->name.value == "get" || method->name.value == "set") && lexer.expectToken(LEFT_BRACKET)) {
        listClose = RIGHT_BRACKET;
        isIndexer = true;
    } else if ((method->name.value == "get" || method->name.value == "set") && lexer.peekToken().kind == RIGHT_BRACKET) {
        // tolerate lexer path where '[' has already been consumed before this point
        listClose = RIGHT_BRACKET;
        isIndexer = true;
    } else if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected '(' after method name\n";
        method->valid = false;
        method->expected = tokenKindsToString({LEFT_PARENTHESIS, LEFT_BRACKET});
        method->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return method;
    }

    while (!isDisplayShortcut && !lexer.expectToken(listClose)) {

        TypeSyntaxExpression typeExpr;
        if (!parseTypeSyntaxExpression(lexer, typeExpr, *method)) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected parameter type\n";
            method->valid = false;
            method->expected = {"parameter type"};
            lexer.rollPosition();
            return method;
        }

        Token paramName = lexer.nextToken();
        bool variadic = false;
        if (paramName.kind == ELLIPSIS) {
            variadic = true;
            paramName = lexer.nextToken();
        }
        if (paramName.kind != IDENTIFIER) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected parameter name\n";
            method->valid = false;
            method->expected = {"parameter name"};
            method->lastToken = paramName;
            lexer.rollPosition();
            return method;
        }

        method->parameters.push_back({typeExpr, paramName, variadic});
        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed method parameter: " << paramName.value << "\n";

        Token sep = lexer.nextToken();
        if (sep.kind == listClose) break;

        if (variadic) {
            method->valid = false;
            method->expected = tokenKindsToString({listClose});
            method->lastToken = sep;
            method->errorMessage = "variadic parameter must be the last parameter";
            lexer.rollPosition();
            return method;
        }

        if (sep.kind != COMMA) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected ',' or ')' in parameter list\n";
            method->valid = false;
            method->expected = tokenKindsToString({COMMA, listClose});
            method->lastToken = sep;
            lexer.rollPosition();
            return method;
        }
    }

    if (isOperatorOverload) {
        if ((method->flags & MemberFlags::Static) == 0) {
            method->valid = false;
            method->expected = {"binary operator overloads must be static"};
            method->lastToken = method->name;
            method->errorMessage = "binary operator overloads must be static";
            lexer.rollPosition();
            return method;
        }
        if (method->parameters.size() != 2 || method->parameters[0].variadic || method->parameters[1].variadic) {
            method->valid = false;
            method->expected = {"two non-variadic parameters"};
            method->lastToken = method->name;
            method->errorMessage = "binary operator overloads must take exactly two non-variadic parameters";
            lexer.rollPosition();
            return method;
        }
    }

    if (isIndexer && method->name.value == "set" && method->parameters.empty()) {
        method->valid = false;
        method->expected = {"set[...] must declare at least one index parameter"};
        method->lastToken = method->name;
        method->errorMessage = "set[...] must declare at least one index parameter; assigned value is available as 'value'";
        lexer.rollPosition();
        return method;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing method body\n";
    auto body = Block::parse(lexer);
    if (!body->valid) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid method body\n";
        method->valid = false;
        method->expected = body->expected;
        method->lastToken = body->lastToken;
        lexer.rollPosition();
        return method;
    }

    method->body = move(body);
    method->valid = true;
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Method parsed successfully: " << method->name.value << "\n";

    lexer.deletePosition();
    return method;
}

Value MethodDefinition::execute(Scope& scope) {

    Function function;
    function.body = body;
    function.parameters = {};
    function.debugName = name.value;
    function.declarationLine = name.line;
    function.declarationCol = name.pos + 1;

    auto childScope = scope.createChild();

    for (auto& param : parameters) {

        FunctionParameter fp;
        fp.name = param.name.value;
        fp.variadic = param.variadic;

        Value typeVal = resolveTypeSyntaxExpression(childScope, param.type, false, "");
        if (typeVal.type != TypeType) {
            return makeClassRegistrationError("unknown type");
        }

        fp.type = get<reference<Type>>(typeVal.value);
        childScope.addVariable(fp.name, Value::Uninitialized(fp.type));

        function.parameters.push_back(fp);
    }

    function.closure = create_reference<Scope>(scope.snapshot());

    return Value(function);
}
