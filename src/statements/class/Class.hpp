#pragma once

#include "../expression/Expression.h"
#include "../variable/VariableDefinition.h"
#include "../Block.hpp"
#include <vector>
#include <unordered_map>

struct ClassDeclarationTypeParameter {

    Token name;
    bool withDefaultValue;
    Token defaultValue;

};

struct FieldDefinition : public Statement {

    Token name;
    int flags;
    reference<Expression> type;
    bool withValue;
    reference<Expression> value;

    static FieldDefinition parse(Lexer& lexer);
    Value execute(Scope& scope) override { return NullValue; };

};

struct LambdaParameter {
    Token type;
    Token name;
    bool variadic = false;
};

struct MethodDefinition : public Expression {

    Token name;
    int flags;
    bool isConstructor = false;
    std::vector<LambdaParameter> parameters;
    reference<Block> body;

    static uref<MethodDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

};

struct ClassDeclaration : public Statement {

    static uref<ClassDeclaration> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    string name;

    std::vector<ClassDeclarationTypeParameter> typeParameters;
    std::vector<FieldDefinition> fields;
    std::unordered_map<string, std::vector<uref<MethodDefinition>>> methods;
    std::vector<uref<MethodDefinition>> constructors;

};
