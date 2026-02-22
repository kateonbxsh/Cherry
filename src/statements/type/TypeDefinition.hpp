#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"
#include "type_syntax.h"

struct TypeDeclarationParameter {
    Token name;
    bool withDefaultValue = false;
    TypeSyntaxExpression defaultValue;
    bool withConstraint = false;
    TypeSyntaxExpression constraintType;
};

struct TypeUnionTerm {
    Token token;
    bool literal = false;
    std::vector<TypeSyntaxExpression> typeArguments;
};

class TypeDefinition : public Statement {
public:
    static uref<TypeDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    string name;
    std::vector<TypeDeclarationParameter> parameters;

    bool usesPredicateSyntax = false;
    TypeSyntaxExpression predicateBaseType;
    Token predicateVariable;
    reference<Expression> predicateExpression;
    reference<Expression> defaultExpression;

    std::vector<TypeUnionTerm> unionTerms;
};
