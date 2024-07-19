#include "VariableDefinition.h"
#include "../expression/Expression.h"

VariableDefinition *VariableDefinition::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varDef = new VariableDefinition();

    auto nextToken = lexer.nextToken();
    if (nextToken.kind == IDENTIFIER) {
        varDef->type = nextToken;
    } else {
        varDef->lastToken = nextToken;
        varDef->expected = {IDENTIFIER};
        varDef->valid = false;
        lexer.rollPosition();
        return varDef;
    }

    nextToken = lexer.nextToken();
    if (nextToken.kind == IDENTIFIER) {
        varDef->name = nextToken;
    } else {
        varDef->lastToken = nextToken;
        varDef->expected = {IDENTIFIER};
        varDef->valid = false;
        lexer.rollPosition();
        return varDef;
    }

    lexer.savePosition();
    nextToken = lexer.nextToken();
    if (nextToken.kind == SEMICOLON) {
        varDef->valid = true;
        varDef->expression = nullptr;
        lexer.deletePosition();
        return varDef;
    }
    lexer.rollPosition();

    auto expression = Expression::parse(lexer);

    if (!expression->valid) {
        varDef->lastToken = expression->lastToken;
        varDef->expected = expression->expected;
        varDef->valid = false;
        delete expression;
        lexer.rollPosition();
        return varDef;
    }

    varDef->expression = expression;

    nextToken = lexer.nextToken();
    if (nextToken.kind == SEMICOLON) {
        varDef->valid = true;
        lexer.deletePosition();
        return varDef;
    }
    
    delete expression;
    varDef->lastToken = nextToken;
    varDef->expected = {SEMICOLON};
    return varDef;

}