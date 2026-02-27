#pragma once

#include <vector>
#include <string>
#include "lexer.h"
#include "statement.h"
#include "scope.h"

struct TypeSyntaxExpression {
    Token name;
    std::vector<TypeSyntaxExpression> arguments;
    bool isUnion = false;
    std::vector<TypeSyntaxExpression> unionMembers;
};

bool parseTypeSyntaxExpression(Lexer& lexer, TypeSyntaxExpression& out, Statement& st);

Value instantiateGenericTypeSyntax(
    reference<Type> baseType,
    const std::vector<reference<Type>>& args,
    bool strictMissing,
    const std::string& errorPrefix
);

Value resolveTypeSyntaxExpression(
    Scope& scope,
    const TypeSyntaxExpression& syntax,
    bool strictMissing,
    const std::string& errorPrefix
);

