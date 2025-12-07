#include "statement.h"
#include "GeneralStatement.h"
#include "variable/VariableDefinition.h"
#include "variable/VariableAffectation.h"
#include <iostream>
#include "macros.h"

uref<Statement> GeneralStatement::parse(Lexer& lexer) {
    
    std::vector<uref<Statement>> invalids = {};

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying VariableDefinition::parse\n";
    auto variableDefinition = VariableDefinition::parse(lexer);
    if (!variableDefinition->valid) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "VariableDefinition invalid\n";
        invalids.push_back(move(variableDefinition));
    } else {
        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "VariableDefinition valid\n";
        return variableDefinition;
    }

    if (DEBUG) std::cout << DEBUG_PREFIX << "Trying VariableAffectation::parse\n";
    auto variableAffectation = VariableAffectation::parse(lexer);
    if (!variableAffectation->valid) {
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "VariableAffectation invalid\n";
        invalids.push_back(move(variableAffectation));
    } else {
        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "VariableAffectation valid\n";
        return variableAffectation;
    }

    // No valid statement
    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "No valid statement found, returning furthest invalid\n";
    auto furthest = getFurthestInvalidStatement(invalids);
    return furthest;

}