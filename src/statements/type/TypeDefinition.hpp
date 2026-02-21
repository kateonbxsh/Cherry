#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"

struct TypeDeclarationParameter {
    Token name;
    bool withDefaultValue = false;
    Token defaultValue;
    bool withConstraint = false;
    Token constraintType;
};

struct TypeUnionTerm {
    Token token;
    bool literal = false;
    std::vector<Token> typeArguments;
};

class TypeDefinition : public Statement {
public:
    static uref<TypeDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    string name;
    std::vector<TypeDeclarationParameter> parameters;

    bool usesPredicateSyntax = false;
    Token predicateBaseType;
    Token predicateVariable;
    reference<Expression> predicateExpression;
    reference<Expression> defaultExpression;

    std::vector<TypeUnionTerm> unionTerms;
};
