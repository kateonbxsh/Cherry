#include "VariableDefinition.h"
#include "statements/expression/Expression.h"
#include "types/type.h"

uref<VariableDefinition> VariableDefinition::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varDef = create_unique<VariableDefinition>();

    if (lexer.expectToken(INFER)) {
        varDef->inferred = true;
    } else {

        Token nextToken = lexer.nextToken();
        if (nextToken.kind == IDENTIFIER) {
            varDef->type = nextToken;
        } else {
            varDef->lastToken = nextToken;
            varDef->expected = {"type name", "class name", "interface name", "enum name", "primitive type"};
            varDef->valid = false;
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

    Value type = scope.getVariable(this->type.value);
    auto name = this->name.value;

    if (!inferred) {
        if (type.type != TypeType) { // wtf am i doing atp
            Value ret;
            ret.thrownException = create_reference<Value>(
                "type value is not a type"
            );
            return ret;
        }
        auto typeType = get<reference<Type>>(type.value);
        if (expression == nullptr) {
            auto value = Value::Uninitialized(typeType);
            scope.setVariable(name, value);
            return value;
        }
    }

    if (this->expression != nullptr) {
        auto value = this->expression->execute(scope);
        scope.setVariable(name, value);
        return value;
    } else {
        
        return NullValue;
    }

}