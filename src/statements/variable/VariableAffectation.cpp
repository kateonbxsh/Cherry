#include "VariableAffectation.h"
#include "statements/expression/Expression.h"
#include "types/type.h"
#include <expressions.h>

unique<VariableAffectation> VariableAffectation::parse(Lexer &lexer) {

    lexer.savePosition();

    auto varAff = create_unique<VariableAffectation>();

    Token next = lexer.nextToken();
    if (next.kind == IDENTIFIER) {
        varAff->name = next;
    } else {
        varAff->valid = false;
        varAff->expected = "variable name";
        varAff->lastToken = next;
        lexer.rollPosition();
        return varAff;
    }

    varAff->selfOperation = NONE;

    next = lexer.nextToken();
    if (isOperator(next)) {
        varAff->selfOperation = next.kind;
    } else {
        lexer.back();
    }

    if (!lexer.expectToken(EQUALS)) {
        varAff->valid = false;
        varAff->expected = "=";
        varAff->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varAff;
    }

    auto potentialExpr = Expression::parse(lexer);
    
    if (!potentialExpr->valid) {
        varAff->valid = false;
        varAff->expected = potentialExpr->expected;
        varAff->lastToken = potentialExpr->lastToken;
        lexer.rollPosition();
        return varAff;
    }

    varAff->expression = move(potentialExpr);

    if (!lexer.expectToken(SEMICOLON)) {
        varAff->valid = false;
        varAff->expected = ";";
        varAff->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return varAff;
    }

    varAff->valid = true;
    return varAff;

}

Value VariableAffectation::execute(Scope& scope) {

    if (!scope.hasVariable(name.value)) {
        return NullValue;
    }

    Value value = expression->execute(scope);
    if (value.thrownException != nullptr) return value;

    if (selfOperation != NONE) {
        value = performOperator(scope.getVariable(name.value), value, selfOperation);
    }
    scope.setVariable(name.value, value);
    return value;

}