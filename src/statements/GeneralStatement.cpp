#include "statement.h"
#include "GeneralStatement.h"
#include "variable/VariableDefinition.h"
#include "variable/VariableAffectation.h"
#include "function/FunctionDeclaration.hpp"
#include "function/FunctionStatement.hpp"
#include "conditional/IfStatement.hpp"
#include <iostream>
#include "macros.h"
#include <functional>

uref<Statement> GeneralStatement::parse(Lexer& lexer) {
    
    std::vector<uref<Statement>> invalids = {};

    using ParserFunc = std::function<uref<Statement>(Lexer&)>;

    // List of parser functions with their names for debug
    std::vector<std::pair<std::string, ParserFunc>> parsers = {
        {"VariableDefinition", [](Lexer& l) { return VariableDefinition::parse(l); }},
        {"VariableAffectation", [](Lexer& l) { return VariableAffectation::parse(l); }},
        {"FunctionDeclaration", [](Lexer& l) { return FunctionDeclaration::parse(l); }},
        {"FunctionStatement", [](Lexer& l) { return FunctionStatement::parse(l); }},
        {"IfStatement", [](Lexer& l) { return IfStatement::parse(l); }}
    };

    for (auto& [name, parser] : parsers) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Trying " << name << "::parse\n";

        auto stmt = parser(lexer);

        if (!stmt->valid) {
            if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << name << " invalid\n";
            invalids.push_back(move(stmt));
        } else {
            if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << name << " valid\n";
            return stmt;
        }
    }

    // No valid statement
    if (DEBUG) std::cout << DEBUG_ERROR_PREFIX << "No valid statement found, returning furthest invalid\n";
    auto furthest = getFurthestInvalidStatement(invalids);
    return furthest;

}