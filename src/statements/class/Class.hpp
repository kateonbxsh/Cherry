#pragma once

#include "../expression/Expression.h"
#include "../variable/VariableDefinition.h"

struct ClassDeclarationTypeParameter {

    Token name;
    bool withDefaultValue;
    reference<Expression> defaultValue; 

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


struct ClassDeclaration : public Statement {

    static uref<ClassDeclaration> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    string name;
    
    std::unordered_map<string, ClassDeclarationTypeParameter> typeParameters;
    std::vector<FieldDefinition> fields;
    std::unordered_map<string, reference<MethodDefinition>> methods;

};

struct MethodParameter {
    Token type;
    Token name;
};

struct MethodDefinition : public Expression {
    
    Token name;
    std::vector<MethodParameter> parameters;
    reference<Block> body;

    static reference<MethodDefinition> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

};
