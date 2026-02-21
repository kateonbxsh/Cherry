#include "Class.hpp"

uref<ClassDeclaration> ClassDeclaration::parse(Lexer& lexer) {

    lexer.savePosition();
    auto klass = create_unique<ClassDeclaration>();

    // class keyword
    if (!lexer.expectToken(CLASS)) {
        klass->valid = false;
        lexer.rollPosition();
        return klass;
    }

    // class name
    Token name = lexer.nextToken();
    if (name.kind != IDENTIFIER) {
        klass->valid = false;
        klass->expected = {"class name"};
        klass->lastToken = name;
        lexer.rollPosition();
        return klass;
    }

    klass->name = name.value;

    /* =========================
       Type parameters
       ========================= */
    if (lexer.expectToken(SMALLER_THAN)) {

        while (true) {

            Token typeName = lexer.nextToken();
            if (typeName.kind != IDENTIFIER) {
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
                auto def = Expression::parse(lexer);
                if (!def->valid) {
                    klass->valid = false;
                    klass->expected = def->expected;
                    klass->lastToken = def->lastToken;
                    lexer.rollPosition();
                    return klass;
                }

                param.withDefaultValue = true;
                param.defaultValue = move(def);
            }

            klass->typeParameters[param.name.value] = move(param);

            Token sep = lexer.nextToken();
            if (sep.kind == BIGGER_THAN) break;

            if (sep.kind != COMMA) {
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
        klass->valid = false;
        klass->expected = {"{"};
        klass->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return klass;
    }

    while (!lexer.expectToken(RIGHT_BRACE)) {

        lexer.savePosition();

        // Try method
        auto method = MethodDefinition::parse(lexer);
        if (method->valid) {
            klass->methods[method->name.value] = move(method);
            lexer.deletePosition();
            continue;
        }

        lexer.rollPosition();
        lexer.savePosition();

        // Try field
        auto field = FieldDefinition::parse(lexer);
        if (field.valid) {
            klass->fields.push_back(field);
            lexer.deletePosition();
            continue;
        }

        lexer.rollPosition();

        // Nothing matched
        klass->valid = false;
        klass->expected = {"field or method"};
        klass->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return klass;
    }

    klass->valid = true;
    lexer.deletePosition();
    return klass;
}

FieldDefinition FieldDefinition::parse(Lexer& lexer) {

    FieldDefinition fieldDef;
    auto token = lexer.peekToken();
    fieldDef.flags = 0;
    fieldDef.withValue = false;
    if (token.kind == PUBLIC || token.kind == PRIVATE || token.kind == PROTECTED) {
        if (token.kind == PUBLIC) fieldDef.flags |= FieldFlags::Public;
        else if (token.kind == PRIVATE) fieldDef.flags |= FieldFlags::Private;
        else fieldDef.flags |= FieldFlags::Protected;
    } else fieldDef.flags |= FieldFlags::Private;

    if (lexer.expectToken(STATIC)) {
        fieldDef.flags |= FieldFlags::Static;
    }

    auto typeExpr = Expression::parse(lexer);
    if (!typeExpr->valid) {
        fieldDef.invalidFrom(*typeExpr);
        return fieldDef;
    }
    fieldDef.type = move(typeExpr);
    
    if (!lexer.expectToken(IDENTIFIER)) {
        fieldDef.invalidExpected({IDENTIFIER}, lexer);
        return fieldDef;
    }

    fieldDef.name = lexer.currentToken();

    if (lexer.expectToken(EQUALS)) {
        auto expr = Expression::parse(lexer);
        if (!expr->valid) {
            fieldDef.invalidFrom(*expr);
            return fieldDef;
        }
        fieldDef.withValue = true;
        fieldDef.value = move(expr);
    }

    if (lexer.expectToken(SEMICOLON)) {
        fieldDef.invalidExpected({SEMICOLON}, lexer);
        return fieldDef;
    }

    fieldDef.valid = true;
    return fieldDef;

}

Value ClassDeclaration::execute(Scope& scope) {

    // Create class type
    auto classType = create_reference<Type>(TypeKind::Class);
    classType->setName(name);

    /* =========================
       Type parameters
       ========================= */
    for (auto& [paramName, param] : typeParameters) {

        TypeParameter tp;
        tp.name = paramName;
        tp.hasDefault = param.withDefaultValue;

        if (param.withDefaultValue) {
            Value def = param.defaultValue->execute(scope);
            if (def.type == TypeType) return Value{};
            if (def.thrownException != nullptr) return def;
            tp.defaultValue = get<reference<Type>>(def.value);
        }

        classType->typeParameters.push_back(tp);
    }


    for (auto& field : fields) {

        Field f;
        f.name = field.name.value;
        f.type = move(field.type);

        if (field.withValue) {
            f.hasDefaultValue = true;
            f.value = move(field.value);
        }

        classType->fields.push_back(f);
    }

    /* =========================
       Methods
       ========================= */
    for (auto& [methodName, methodDef] : methods) {

        Value methodValue = methodDef->execute(scope);
        if (methodValue.thrownException != nullptr) return methodValue;

        if (classType->methods. > 0)

        Method method;
        method.overloads.push_back(get<Function>(methodValue.value));

        [methodName] = method;
    }

    /* =========================
       Register class in scope
       ========================= */
    scope.addVariable(name, Value(classType));

    return Value(classType);
}


reference<MethodDefinition> MethodDefinition::parse(Lexer& lexer) {

    lexer.savePosition();
    auto method = create_unique<MethodDefinition>();


    Token name = lexer.nextToken();
    if (name.kind != IDENTIFIER) {
        method->valid = false;
        lexer.rollPosition();
        return method;
    }

    method->name = name;

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        method->valid = false;
        method->expected = {"("};
        method->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return method;
    }

    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {

        Token type = lexer.nextToken();
        if (type.kind != IDENTIFIER) {
            method->valid = false;
            method->expected = {"parameter type"};
            method->lastToken = type;
            lexer.rollPosition();
            return method;
        }

        Token paramName = lexer.nextToken();
        if (paramName.kind != IDENTIFIER) {
            method->valid = false;
            method->expected = {"parameter name"};
            method->lastToken = paramName;
            lexer.rollPosition();
            return method;
        }

        method->parameters.push_back({type, paramName});

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) break;

        if (sep.kind != COMMA) {
            method->valid = false;
            method->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            method->lastToken = sep;
            lexer.rollPosition();
            return method;
        }
    }

    auto body = Block::parse(lexer);
    if (!body->valid) {
        method->valid = false;
        method->expected = body->expected;
        method->lastToken = body->lastToken;
        lexer.rollPosition();
        return method;
    }

    method->body = move(body);
    method->valid = true;

    lexer.deletePosition();
    return method;
}
