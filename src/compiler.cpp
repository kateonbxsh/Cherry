#include <utility>
#include <algorithm>
#include <set>
#include <sstream>

#include "compiler.h"
#include "statement.h"
#include "statements/definition/VariableDefinition.h"
#include "statements/VariableAffectation.h"
#include "statements/Return.h"
#include "statements/IfStatement.h"
#include "statements/FunctionCall.h"

Compiler::Compiler(Lexer& inputLexer) {
    lexer = inputLexer;
}

Block Compiler::parse() {

    //clear callstack
    lexer.initReader();

    //init callstack
    Scope scope;

    return parseInternal(scope);

}

Block Compiler::parseInternal(Scope scope) {

    std::vector<TokenKind> generalExpected = {IDENTIFIER, METHOD, IF, RETURN, END_OF_FEED, RIGHT_BRACKET};
    std::vector<TokenKind> expected = generalExpected;
    std::vector<Type> types = scope.getTypes();
    std::string currentVariableName;
    std::string currentVariableType;
    IfStatement* currentIfStatement;
    Value currentValue;

    Block finalBlock;
    std::stack<Block*> blockStack;
    blockStack.push(&finalBlock);

    std::stack<CompilerState> stateStack;
    stateStack.push(GLOBAL);

    bool awaitingOneStatement = false;

    while(lexer.hasNextToken()) {

        Token token = lexer.nextToken();
        TokenKind kind = token.kind; std::string value = token.value;
        Block* currentBlock = blockStack.top();
        CompilerState& state = stateStack.top();

        expected.clear();
        switch (state) {

            case AWAITING_BLOCK:
            case GLOBAL: {
                awaitingOneStatement = state == AWAITING_BLOCK;
                if (kind == IDENTIFIER) {
                    if (lexer.peekToken().kind == LEFT_BRACE) {
                        lexer.back();
                        auto fCall = new FunctionCall;
                        fCall->expression = parseExpression();
                        currentBlock->statements.push_back(fCall);
                        state = AWAITING_SEMICOLON;
                        continue;
                    }
                    state = RET_VAR_NAME;
                    currentVariableName = value;
                    expected = {IDENTIFIER, EQUALS};
                    continue;
                }
                if (kind == RETURN) {
                    TokenList expression = parseExpression();
                    auto returnStatement = new Return;
                    returnStatement->expression = expression;
                    currentBlock->statements.push_back(returnStatement);
                    state = AWAITING_SEMICOLON;
                    continue;
                }
                if (kind == RIGHT_BRACKET) {
                    if (blockStack.size() < 2) {
                        std::cerr << "wtf?";
                        return finalBlock;
                    }
                    blockStack.pop();
                    stateStack.pop();
                    continue;
                }
                if (kind == LEFT_BRACKET && state == AWAITING_BLOCK) {
                    state = GLOBAL;
                    awaitingOneStatement = false;
                    continue;
                }
                if (state == AWAITING_BLOCK) break;
                if (kind == END_OF_FEED) {
                    return finalBlock;
                }
                if (kind == IF || kind == UNLESS) {
                    auto ifStatement = new IfStatement;
                    auto subIfStatement = new IfStatement::SubIfStatement;
                    subIfStatement->isUnless = (kind == UNLESS);
                    subIfStatement->expression = parseExpression();
                    ifStatement->subStatements.push_back(subIfStatement);
                    currentBlock->statements.push_back(ifStatement);
                    currentIfStatement = ifStatement;
                    state = RET_ELSE;

                    stateStack.push(AWAITING_BLOCK);
                    blockStack.push(&subIfStatement->block);

                    continue;
                }
            }

            case RET_ELSE: {
                if (kind == ELSE) {
                    Token upcomingToken = lexer.nextToken();
                    auto subIfStatement = new IfStatement::SubIfStatement;
                    subIfStatement->isElse = true;
                    if (upcomingToken.kind == IF || upcomingToken.kind == UNLESS) {
                        subIfStatement->isUnless = (upcomingToken.kind == UNLESS);
                        subIfStatement->elseHasExpression = true;
                        subIfStatement->expression = parseExpression();
                    } else {
                        subIfStatement->elseHasExpression = false;
                        lexer.back();
                        state = GLOBAL;
                    }
                    currentIfStatement->subStatements.push_back(subIfStatement);

                    blockStack.push(&subIfStatement->block);
                    stateStack.push(AWAITING_BLOCK);
                    continue;
                }
                state = GLOBAL;
                lexer.back();
                continue;
            }

            case RET_VAR_NAME: {
                if (kind == IDENTIFIER) {
                    currentVariableType = currentVariableName;
                    currentVariableName = value;
                    state = RET_VAR_EQUAL;
                    continue;
                } else if (kind == EQUALS) {
                    TokenList expression = parseExpression();
                    auto affectation = new VariableAffectation;
                    affectation->name = currentVariableName;
                    affectation->expression = expression;
                    currentBlock->statements.push_back(affectation);
                    state = AWAITING_SEMICOLON;
                    continue;
                }
            }

            case RET_VAR_EQUAL: {
                auto definition = new Expression;
                definition->name = currentVariableName;
                definition->type = currentVariableType;
                if (kind == SEMICOLON) {
                    state = GLOBAL;
                    currentBlock->statements.push_back(definition);
                    continue;
                }
                TokenList expression = parseExpression();
                definition->expression = expression;
                currentBlock->statements.push_back(definition);
                state = AWAITING_SEMICOLON;
                continue;
            }

            case AWAITING_SEMICOLON: {
                if (kind == SEMICOLON) {
                    if (awaitingOneStatement) {
                        stateStack.pop();
                        blockStack.pop();
                        continue;
                    }
                    state = GLOBAL;
                }
                continue;
            }

        }

        std::cerr << "Unexpected token " << token.value << " at pos: " << token.pos << ", line: " << token.line << std::endl;
        std::string input = lexer.getParseData();
        std::istringstream iss(input);
        std::vector<std::string> subs;
        std::string sub;
        while(std::getline(iss, sub, '\n')) {
            subs.push_back(sub);
        }
        std::cerr << subs[token.line - 1] << std::endl;
        for(int i = 0; i < token.pos - 1; i++) std::cerr << " ";
        for(int i = 0; i < token.value.length(); i++) std::cerr << "^";
        std::cerr << std::endl;
        return finalBlock;

    }

    return finalBlock;
}


