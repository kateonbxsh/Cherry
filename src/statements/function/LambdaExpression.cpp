#include "LambdaExpression.hpp"
#include "types/function.h"

uref<Expression> MethodDefinition::parse(Lexer& lexer) {

    lexer.savePosition();
    auto lambda = create_unique<MethodDefinition>();

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        lambda->valid = false;
        lexer.rollPosition();
        return lambda;
    }

    // parameters
    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {

        Token type = lexer.nextToken();
        if (type.kind != IDENTIFIER) {
            lambda->valid = false;
            lambda->expected = {"parameter type"};
            lambda->lastToken = type;
            lexer.rollPosition();
            return lambda;
        }

        Token name = lexer.nextToken();
        if (name.kind != IDENTIFIER) {
            lambda->valid = false;
            lambda->expected = {"parameter name"};
            lambda->lastToken = name;
            lexer.rollPosition();
            return lambda;
        }

        lambda->parameters.push_back({type, name});

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) break;

        if (sep.kind != COMMA) {
            lambda->valid = false;
            lambda->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            lambda->lastToken = sep;
            lexer.rollPosition();
            return lambda;
        }
    }

    // expect =>
    Token arrow = lexer.nextToken();
    if (arrow.kind != ARROW) {   // token for =>
        lambda->valid = false;
        lambda->expected = {"=>"};
        lambda->lastToken = arrow;
        lexer.rollPosition();
        return lambda;
    }

    // body
    auto body = Block::parse(lexer);
    if (!body->valid) {
        lambda->valid = false;
        lambda->expected = body->expected;
        lambda->lastToken = body->lastToken;
        lexer.rollPosition();
        return lambda;
    }

    lambda->body = move(body);
    lambda->valid = true;

    lexer.deletePosition();
    return lambda;
}

Value MethodDefinition::execute(Scope& scope) {

    Function function;
    function.body = body;
    function.parameters = {};

    auto childScope = Scope(scope);

    for (auto& param : parameters) {

        FunctionParameter fp;
        fp.name = param.name.value;

        Value typeVal = childScope.getVariable(param.type.value);
        if (typeVal.type != TypeType) {
            Value exc;
            exc.thrownException = create_reference<Value>(Value("unknown type"));
            return exc;
        }

        fp.type = get<reference<Type>>(typeVal.value);
        childScope.addVariable(fp.name, Value::Uninitialized(fp.type));

        function.parameters.push_back(fp);
    }

    function.closure = create_reference<Scope>(scope);

    return Value(function);
}
