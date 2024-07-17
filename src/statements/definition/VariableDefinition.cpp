#include "VariableDefinition.h"

VariableDefinition *VariableDefinition::parse(Lexer &lexer) {
    lexer.savePosition();

    auto varDef = new VariableDefinition();
    Token varType, varName, varExpression;

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

}