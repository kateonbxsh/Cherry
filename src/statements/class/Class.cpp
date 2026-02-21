#include "Class.hpp"

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

            if (lexer.expectToken(EQUALS)) {
                if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing type parameter default value\n";
                Token def = lexer.nextToken();
                if (def.kind != IDENTIFIER) {
                    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Invalid type parameter default type\n";
                    klass->valid = false;
                    klass->expected = {"type identifier"};
                    klass->lastToken = def;
                    lexer.rollPosition();
                    return klass;
                }

                param.withDefaultValue = true;
                param.defaultValue = def;
                seenDefault = true;
            } else if (seenDefault) {
                if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Non-default type parameter after default\n";
                klass->valid = false;
                klass->expected = {"type parameter with default value"};
                klass->lastToken = typeName;
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
        DEBUG_TABS--;
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
        klass->expected = {"field or method"};
        klass->lastToken = lexer.nextToken();
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
    while (true) {
        Token token = lexer.peekToken();

        if (token.kind == PUBLIC || token.kind == PRIVATE || token.kind == PROTECTED) {
            hasAccessModifier = true;
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

    /* =========================
       Type parameters
       ========================= */
    for (auto& param : typeParameters) {
        auto paramName = param.name.value;
        if (DEBUG) std::cout << DEBUG_PREFIX << "Resolving type parameter: " << paramName << "\n";

        TypeParameter tp;
        tp.name = paramName;
        tp.hasDefault = param.withDefaultValue;

        if (param.withDefaultValue) {
            Value def = classScope.getVariable(param.defaultValue.value);
            if (def.thrownException != nullptr) return def;
            if (def.type != TypeType) return Value{};
            tp.defaultValue = get<reference<Type>>(def.value);
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Resolved type parameter default: " << paramName << "\n";
        }

        classType->typeParameters.push_back(tp);
        classType->typeBindings[paramName] = nullptr;

        auto placeholderType = create_reference<Type>(TypeKind::Dynamic);
        placeholderType->setName(paramName);
        classScope.addVariable(paramName, Value(placeholderType));
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
                    Value err;
                    err.thrownException = create_reference<Value>(
                        Value("static field type is not a type")
                    );
                    return err;
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

    /* =========================
       Constructor
       ========================= */
    if (!constructors.empty()) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Registering constructors\n";

        Method constructor;
        constructor.flags = constructors.front()->flags;

        for (auto& constructorDef : constructors) {
            Value constructorValue = constructorDef->execute(classScope);
            if (constructorValue.thrownException != nullptr) return constructorValue;

            constructor.overloads.push_back(get<Function>(constructorValue.value));
        }

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
            Value methodValue = methodDef->execute(classScope);
            if (methodValue.thrownException != nullptr) return methodValue;

            method.overloads.push_back(get<Function>(methodValue.value));
        }

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
    while (true) {
        Token token = lexer.peekToken();

        if (token.kind == PUBLIC || token.kind == PRIVATE || token.kind == PROTECTED) {
            hasAccessModifier = true;
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
    if (name.kind != IDENTIFIER) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "No method name found\n";
        method->valid = false;
        lexer.rollPosition();
        return method;
    }

    method->name = name;
    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing method: " << method->name.value << "\n";

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected '(' after method name\n";
        method->valid = false;
        method->expected = {"("};
        method->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return method;
    }

    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {

        Token type = lexer.nextToken();
        if (type.kind != IDENTIFIER) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected parameter type\n";
            method->valid = false;
            method->expected = {"parameter type"};
            method->lastToken = type;
            lexer.rollPosition();
            return method;
        }

        Token paramName = lexer.nextToken();
        if (paramName.kind != IDENTIFIER) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected parameter name\n";
            method->valid = false;
            method->expected = {"parameter name"};
            method->lastToken = paramName;
            lexer.rollPosition();
            return method;
        }

        method->parameters.push_back({type, paramName});
        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed method parameter: " << paramName.value << "\n";

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) break;

        if (sep.kind != COMMA) {
            if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "Expected ',' or ')' in parameter list\n";
            method->valid = false;
            method->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            method->lastToken = sep;
            lexer.rollPosition();
            return method;
        }
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

    auto childScope = Scope(scope);

    for (auto& param : parameters) {

        FunctionParameter fp;
        fp.name = param.name.value;

        Value typeVal = childScope.getVariable(param.type.value);
        if (typeVal.type != TypeType) {
            Value exc;
            exc.thrownException = create_reference<Value>(Value("unknown type"));
            return exc;
        }

        fp.type = get<reference<Type>>(typeVal.value);
        childScope.addVariable(fp.name, Value::Uninitialized(fp.type));

        function.parameters.push_back(fp);
    }

    function.closure = create_reference<Scope>(scope);

    return Value(function);
}

