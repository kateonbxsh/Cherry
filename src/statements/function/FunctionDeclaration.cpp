#include "FunctionDeclaration.hpp"

uref<FunctionDeclaration> FunctionDeclaration::parse(Lexer& lexer) {

    lexer.savePosition();
    auto funcDecl = create_unique<FunctionDeclaration>();

    // Expect "method" keyword
    if (!lexer.expectToken(METHOD)) {
        funcDecl->valid = false;
        funcDecl->expected = tokenKindsToString({METHOD});
        funcDecl->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return funcDecl;
    }

    // Expect function name (identifier)
    Token next = lexer.nextToken();
    if (next.kind != IDENTIFIER) {
        funcDecl->valid = false;
        funcDecl->expected = {"function name"};
        funcDecl->lastToken = next;
        lexer.rollPosition();
        return funcDecl;
    }
    funcDecl->name = next;

    // Expect "("
    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        funcDecl->valid = false;
        funcDecl->expected = tokenKindsToString({LEFT_PARENTHESIS});
        funcDecl->lastToken = lexer.nextToken();
        lexer.rollPosition();
        return funcDecl;
    }

    // Parse parameters (identifiers separated by commas)
    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        Token type = lexer.nextToken();
        Parameter parameter;
        if (type.kind != IDENTIFIER) {
            funcDecl->valid = false;
            funcDecl->expected = {"parameter type"};
            funcDecl->lastToken = type;
            lexer.rollPosition();
            return funcDecl;
        }
        parameter.type = type;
        Token name = lexer.nextToken();
        if (name.kind != IDENTIFIER) {
            funcDecl->valid = false;
            funcDecl->expected = {"parameter name"};
            funcDecl->lastToken = name;
            lexer.rollPosition();
            return funcDecl;
        }
        parameter.name = name;
        funcDecl->parameters.push_back(parameter);

        // Either ")" or "," next
        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) {
            break;
        } else if (sep.kind != COMMA) {
            funcDecl->valid = false;
            funcDecl->expected = tokenKindsToString({COMMA, RIGHT_PARENTHESIS});
            funcDecl->lastToken = sep;
            lexer.rollPosition();
            return funcDecl;
        }
    }
    // Parse function body (GlobalBlock)
    auto bodyBlock = Block::parse(lexer);
    if (!bodyBlock->valid) {
        funcDecl->valid = false;
        funcDecl->expected = bodyBlock->expected;
        funcDecl->lastToken = bodyBlock->lastToken;
        lexer.rollPosition();
        return funcDecl;
    }

    funcDecl->body = move(bodyBlock);
    funcDecl->valid = true;
    return funcDecl;
}

Value FunctionDeclaration::execute(Scope& scope) {
    // Store the function in the scope
    //scope.setFunction(name.value, create_unique<FunctionDeclaration>(*this));
    return NullValue;
}
