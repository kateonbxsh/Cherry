#include "VariableDefinition.h"
#include "statements/expression/Expression.h"
#include "types/type.h"
#include "runtime_exception.h"
#include "type_syntax.h"

namespace {

Value makeVarDefError(const std::string& message) {
    return makeThrown("TypeException", message);
}

Value coerceToDeclaredType(const Value& source, const reference<Type>& targetType) {
    Value coerced = source;
    if (targetType == nullptr) return coerced;
    if (targetType->kind == TypeKind::Dynamic) return coerced;
    coerced.type = targetType;
    if (std::holds_alternative<ClassInstance>(coerced.value)) {
        auto instance = get<ClassInstance>(coerced.value);
        instance.classType = targetType;
        coerced.value = instance;
    }
    return coerced;
}

}

uref<VariableDefinition> VariableDefinition::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varDef = create_unique<VariableDefinition>();

    if (lexer.expectToken(INFER)) {
        varDef->inferred = true;
    } else {

        if (!parseTypeSyntaxExpression(lexer, varDef->type, *varDef)) {
            lexer.rollPosition();
            return varDef;
        }

    }

    Token nextToken = lexer.nextToken();
    if (nextToken.kind == IDENTIFIER) {
        varDef->name = nextToken;
    } else {
        varDef->lastToken = nextToken;
        varDef->expected = {"variable name"};
        varDef->valid = false;
        lexer.rollPosition();
        return varDef;
    }
    
    if (!varDef->inferred && lexer.expectToken(SEMICOLON)) {
        varDef->valid = true;
        varDef->expression = nullptr;
        lexer.deletePosition();
        return varDef;
    } else if (!lexer.expectToken(EQUALS)) {
        varDef->valid = false;
        varDef->expression = nullptr;
        varDef->expected = tokenKindsToString({EQUALS, SEMICOLON});
        varDef->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varDef;
    }

    auto parsedExpression = Expression::parse(lexer);

    if (!parsedExpression->valid) {
        varDef->lastToken = parsedExpression->lastToken;
        varDef->expected = parsedExpression->expected;
        varDef->valid = false;
        lexer.rollPosition();
        return varDef;
    }

    varDef->expression = move(parsedExpression);

    if (lexer.expectToken(SEMICOLON)) {
        varDef->valid = true;
        lexer.deletePosition();
        return varDef;
    }
    
    varDef->valid = false;
    varDef->lastToken = lexer.peekToken();
    varDef->expected = tokenKindsToString({SEMICOLON});
    lexer.rollPosition();
    return varDef;

}

Value VariableDefinition::execute(Scope& scope) {

    auto name = this->name.value;

    if (!inferred) {
        Value type = resolveTypeSyntaxExpression(scope, this->type, false, "");
        if (type.thrownException != nullptr) return type;
        if (type.type != TypeType) return makeVarDefError("type value is not a type");
        auto typeType = get<reference<Type>>(type.value);

        if (expression == nullptr) {
            auto value = Value::Uninitialized(typeType);
            scope.addVariable(name, value);
            return value;
        }

        auto value = this->expression->execute(scope);
        if (value.thrownException != nullptr) return value;
        if (!typeType->assignableFrom(value)) {
            return makeVarDefError("value is not assignable to type " + typeType->getName());
        }
        value = coerceToDeclaredType(value, typeType);
        if (value.type == FunctionType) {
            auto fn = get<Function>(value.value);
            if (fn.debugName.empty() || fn.debugName == "<lambda>") {
                fn.debugName = name;
                value.value = fn;
            }
        }
        scope.addVariable(name, value);
        return value;
    }

    if (this->expression != nullptr) {
        auto value = this->expression->execute(scope);
        if (value.type == FunctionType) {
            auto fn = get<Function>(value.value);
            if (fn.debugName.empty() || fn.debugName == "<lambda>") {
                fn.debugName = name;
                value.value = fn;
            }
        }
        scope.addVariable(name, value);
        return value;
    } else {
        
        return NullValue;
    }

}
