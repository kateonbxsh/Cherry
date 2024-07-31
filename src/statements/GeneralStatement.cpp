#include "statement.h"
#include "GeneralStatement.h"
#include "variable/VariableDefinition.h"
#include "variable/VariableAffectation.h"

unique<Statement> GeneralStatement::parse(Lexer& lexer) {
    
    std::vector<unique<Statement>> invalids = {};

    auto variableDefinition = VariableDefinition::parse(lexer);
    if (!variableDefinition->valid) {
        invalids.push_back(move(variableDefinition));
    } else {
        return variableDefinition;
    }

    auto variableAffectation = VariableAffectation::parse(lexer);
    if (!variableAffectation->valid) {
        invalids.push_back(move(variableDefinition));
    } else {
        return variableAffectation;
    }

    //No valid statement
    auto furthest = getFurthestInvalidStatement(invalids);
    return furthest;

}