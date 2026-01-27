#include "statement.h"
#include "GeneralStatement.h"
#include "variable/VariableDefinition.h"
#include "variable/VariableAffectation.h"
#include "function/ExpressionStatement.hpp"
#include "conditional/IfStatement.hpp"
#include "function/ReturnStatement.hpp"
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
        {"IfStatement", [](Lexer& l) { return IfStatement::parse(l); }},
        {"ExpressionStatement", [](Lexer& l) { return ExpressionStatement::parse(l); }},
        {"ReturnStatement", [](Lexer& l) { return ReturnStatement::parse(l); }}
    };

    for (auto& [name, parser] : parsers) {
        if (DEBUG) std::cout << DEBUG_PREFIX << "Trying " << name << "::parse\n";

        DEBUG_TABS++;
        auto stmt = parser(lexer);
        DEBUG_TABS--;

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