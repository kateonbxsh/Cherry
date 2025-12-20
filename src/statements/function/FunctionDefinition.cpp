#include "FunctionDefinition.hpp"
#include "types/function.h"

uref<FunctionDefinition> FunctionDefinition::parse(Lexer& lexer) {

    lexer.savePosition();
    auto funcDecl = create_unique<FunctionDefinition>();

    // Expect "method" keyword
    if (!lexer.expectToken(FUNCTION)) {
        funcDecl->valid = false;
        funcDecl->expected = tokenKindsToString({FUNCTION});
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

    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing left parenthesis" << std::endl;

    // Expect "("
    if (!lexer.expectToken(LEFT_PARENTHESIS)) {
        funcDecl->valid = false;
        funcDecl->expected = tokenKindsToString({LEFT_PARENTHESIS});
        funcDecl->lastToken = lexer.nextToken();
        lexer.rollPosition();
        if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "No left paren" << std::endl;
        return funcDecl;
    }

    // Parse parameters (identifiers separated by commas)
    int i = 1;
    while (!lexer.expectToken(RIGHT_PARENTHESIS)) {
        Token type = lexer.nextToken();
        Parameter parameter;
        if (DEBUG) std::cout << DEBUG_PREFIX << "Parameter #" << (i++) << std::endl;
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
        if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parameter name: " << name.value << ", type: " << type.value << std::endl;

        // Either ")" or "," next
        Token sep = lexer.nextToken();
        if (sep.kind == RIGHT_PARENTHESIS) {
            if (DEBUG) std::cout << DEBUG_WARNING_PREFIX << "Right parenthesis, ending..." << std::endl;
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
    if (DEBUG) std::cout << DEBUG_PREFIX << "Parsing code block" << std::endl;
    DEBUG_TABS++;
    auto bodyBlock = Block::parse(lexer);
    DEBUG_TABS--;
    if (!bodyBlock->valid) {
        funcDecl->valid = false;
        funcDecl->expected = bodyBlock->expected;
        funcDecl->lastToken = bodyBlock->lastToken;
        lexer.rollPosition();
        return funcDecl;
    }

    funcDecl->body = move(bodyBlock);
    funcDecl->valid = true;
    lexer.deletePosition();
    if (DEBUG) std::cout << DEBUG_SUCCESS_PREFIX << "Parsed function: " << funcDecl->name.value << ", with " << funcDecl->parameters.size() << " parameters" << std::endl;
    return funcDecl;
}

Value FunctionDefinition::execute(Scope& scope) {
    
    Function function;
    function.body = body;
    function.parameters = {};
    auto childScope = Scope(scope);
    for(auto& param : parameters) {
        FunctionParameter functionParameter;
        functionParameter.name = param.name.value;
        Value type = childScope.getVariable(param.type.value);
        if (type.type != TypeType) {
            Value exc;
            exc.thrownException = create_reference<Value>(Value("unknown type"));
            return exc;
        }
        functionParameter.type = get<reference<Type>>(type.value);
        childScope.setVariable(param.name.value, Value::Uninitialized(functionParameter.type));
        function.parameters.push_back(functionParameter);
    }
    function.closure = create_reference<Scope>(scope);
    auto f = Value(function);
    scope.setVariable(name.value, f);
    return f;
}
