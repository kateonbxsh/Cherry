#include "VariableDefinition.h"
#include "statements/expression/Expression.h"
#include "types/type.h"

VariableDefinition *VariableDefinition::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varDef = new VariableDefinition();

    if (lexer.expectToken(LET)) {
        varDef->inferred = true;
    } else {

        Token nextToken = lexer.nextToken();
        if (nextToken.kind == IDENTIFIER) {
            varDef->type = nextToken;
        } else {
            varDef->lastToken = nextToken;
            varDef->expected = {IDENTIFIER};
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
        varDef->expected = {IDENTIFIER};
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
        varDef->expected = {EQUALS};
        varDef->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varDef;
    }

    Expression* parsedExpression = Expression::parse(lexer);

    if (!parsedExpression->valid) {
        varDef->lastToken = parsedExpression->lastToken;
        varDef->expected = parsedExpression->expected;
        varDef->valid = false;
        delete parsedExpression;
        lexer.rollPosition();
        return varDef;
    }

    varDef->expression = parsedExpression;

    if (lexer.expectToken(SEMICOLON)) {
        varDef->valid = true;
        lexer.deletePosition();
        return varDef;
    }
    
    delete parsedExpression;
    varDef->valid = false;
    varDef->lastToken = nextToken;
    varDef->expected = {SEMICOLON};
    return varDef;

}

Value VariableDefinition::execute(Scope& scope) {

    Type* type = scope.getType(this->type.value);
    auto name = this->name.value;

    if (this->expression != nullptr) {
        auto value = this->expression->execute(scope);
        scope.setVariable(name, value);
        return value;
    } else {
        scope.setVariable(name, Value(type, nullptr));
        return NullValue;
    }

}