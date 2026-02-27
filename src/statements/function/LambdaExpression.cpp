#include "LambdaExpression.hpp"
#include "types/function.h"
#include "runtime_exception.h"

namespace {

struct LambdaExpressionReturnStatement final : public Statement {
    explicit LambdaExpressionReturnStatement(uref<Expression> expr) : expression(move(expr)) {}

    Value execute(Scope& scope) override {
        Value value = expression->execute(scope);
        if (value.thrownException != nullptr) return value;
        value.returning = true;
        return value;
    }

    uref<Expression> expression;
};

}

uref<Expression> LambdaDefinition::parse(Lexer& lexer) {

    lexer.savePosition();
    auto lambda = create_unique<LambdaDefinition>();

    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        lambda->valid = false;
        lexer.rollPosition();
        return lambda;
    }

    // parameters
    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {

        Token type = lexer.nextToken();
        if (type.kind != IDENTIFIER && type.kind != TYPE) {
            lambda->valid = false;
            lambda->expected = {"parameter type"};
            lambda->lastToken = type;
            lexer.rollPosition();
            return lambda;
        }

        Token name = lexer.nextToken();
        bool variadic = false;
        if (name.kind == ELLIPSIS) {
            variadic = true;
            name = lexer.nextToken();
        }
        if (name.kind != IDENTIFIER) {
            lambda->valid = false;
            lambda->expected = {"parameter name"};
            lambda->lastToken = name;
            lexer.rollPosition();
            return lambda;
        }

        lambda->parameters.push_back({type, name, variadic});

        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) break;

        if (variadic) {
            lambda->valid = false;
            lambda->expected = tokenKindsToString({RIGHT_PARENTHESIS});
            lambda->lastToken = sep;
            lambda->errorMessage = "variadic parameter must be the last parameter";
            lexer.rollPosition();
            return lambda;
        }

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

    // body: either block or expression (implicit return)
    auto blockBody = Block::parse(lexer);
    if (blockBody->valid) {
        lambda->body = move(blockBody);
        lambda->valid = true;
        lexer.deletePosition();
        return lambda;
    }

    auto expressionBody = Expression::parse(lexer);
    if (!expressionBody->valid) {
        lambda->valid = false;
        lambda->expected = expressionBody->expected;
        lambda->lastToken = expressionBody->lastToken;
        lexer.rollPosition();
        return lambda;
    }

    auto synthesizedBody = create_unique<Block>();
    synthesizedBody->valid = true;
    synthesizedBody->statements.push_back(create_unique<LambdaExpressionReturnStatement>(move(expressionBody)));
    lambda->body = move(synthesizedBody);
    lambda->valid = true;

    lexer.deletePosition();
    return lambda;
}

Value LambdaDefinition::execute(Scope& scope) {

    Function function;
    function.body = body;
    function.parameters = {};
    function.debugName = "<lambda>";

    auto closureScope = create_reference<Scope>(scope.snapshot());

    for (auto& param : parameters) {

        FunctionParameter fp;
        fp.name = param.name.value;
        fp.variadic = param.variadic;

        Value typeVal = closureScope->getVariable(param.type.value);
        if (typeVal.type != TypeType) {
            return makeThrown("TypeException", "unknown type");
        }

        fp.type = get<reference<Type>>(typeVal.value);
        if (fp.type == TypeType) {
            auto typePlaceholder = create_reference<Type>(TypeKind::Dynamic);
            typePlaceholder->setName(fp.name);
            closureScope->addVariable(fp.name, Value(typePlaceholder));
        } else {
            closureScope->addVariable(fp.name, Value::Uninitialized(fp.type));
        }

        function.parameters.push_back(fp);
    }

    function.closure = closureScope;

    return Value(function);
}
