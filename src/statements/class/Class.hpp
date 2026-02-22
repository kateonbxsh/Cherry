#pragma once

#include "../expression/Expression.h"
#include "../variable/VariableDefinition.h"
#include "../Block.hpp"
#include "type_syntax.h"
#include <vector>
#include <unordered_map>

struct ClassDeclarationTypeParameter {

    Token name;
    bool withDefaultValue;
    TypeSyntaxExpression defaultValue;
    bool withConstraint = false;
    TypeSyntaxExpression constraintType;

};

struct MethodDefinition;

struct FieldDefinition : public Statement {

    Token name;
    int flags;
    reference<Expression> type;
    bool withValue;
    reference<Expression> value;

    static FieldDefinition parse(Lexer& lexer);
    Value execute(Scope& scope) override { return NullValue; };

};

struct ClassDeclaration : public Statement {

    static uref<ClassDeclaration> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    string name;
    bool hasBaseType = false;
    TypeSyntaxExpression baseType;

    std::vector<ClassDeclarationTypeParameter> typeParameters;
    std::vector<FieldDefinition> fields;
    std::unordered_map<string, std::vector<uref<MethodDefinition>>> methods;
    std::vector<uref<MethodDefinition>> constructors;

};

struct MethodParameter {
    TypeSyntaxExpression type;
    Token name;
    bool variadic = false;
};

struct MethodDefinition : public Expression {

    Token name;
    int flags;
    bool isConstructor = false;
    std::vector<MethodParameter> parameters;
    reference<Block> body;

    static uref<MethodDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

};
