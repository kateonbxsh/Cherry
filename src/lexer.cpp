#include <regex>
#include "lexer.h"

Lexer::Lexer() = default;
Lexer::Lexer(std::string data) {

    static std::regex nameRegex(R"(\b[a-zA-Z_-][a-zA-Z0-9_-]*\b)");
    static std::regex intRegex(R"(-?[0-9]+)");
    static std::regex floatRegex(R"(-?[0-9]*\.[0-9]+)");

    defineCharKinds();

    parseData = data;
    std::string currentReadValue;
    int i, pos = 0, line = 1, currentPos, currentLine;
    bool inString = false, isName = false, isNumber = false; std::string currentString;
    for(i = 0; i < data.length(); (i++, pos++)) {
        std::string currentChar = std::string(1, data[i]);
        TokenKind kindOfChar = tokenMap[currentChar];
        if (!inString && kindOfChar == QUOTE) {
            inString = true;
            currentString = "";
            continue;
        } else if (inString && kindOfChar == QUOTE) {
            inString = false;
            tokens.push_back({STRING, currentString});
            continue;
        } else if (inString) {
            currentString.append(currentChar);
            continue;
        }
        if (currentReadValue.empty() && kindOfChar == WHITESPACE) {
            if (currentChar == "\n") { pos = 0; line++; }
            continue;
        }
        if (
                (isName && !std::regex_match(currentReadValue + currentChar, nameRegex)) ||
                (isNumber && !std::regex_match(currentReadValue + currentChar, intRegex) &&
                !std::regex_match(currentReadValue + currentChar, floatRegex)))
        {
            isName = false; isNumber = false;
            Token result = convertToken(currentReadValue);
            result.line = currentLine; result.pos = currentPos;
            tokens.push_back(result);
            currentReadValue.clear();
            if (kindOfChar == WHITESPACE) continue;
        }
        if (kindOfChar != WHITESPACE) {
            if (currentReadValue.empty()) {
                currentLine = line;
                currentPos = pos;
            }
            currentReadValue.append(currentChar);
            if (std::regex_match(currentReadValue, nameRegex)) {
                isName = true;
            }
            if (std::regex_match(currentReadValue, intRegex) || std::regex_match(currentReadValue, floatRegex)) {
                isNumber = true;
            }
        }
        if (i < data.length() - 1 && tokenMap[currentReadValue + data[i+1]] != NONE) continue;
        if (tokenMap[currentReadValue] != NONE || kindOfChar == WHITESPACE) {
            isNumber = false; isName = false;
            Token result = convertToken(currentReadValue);
            result.line = line; result.pos = pos;
            tokens.push_back(result);
            currentReadValue.clear();
        }
    }

};

Token Lexer::nextToken() {
    if (reader < tokens.size()) {
        return tokens.at(reader++);
    }
    reader++;
    return {END_OF_FEED, ""};
}

void Lexer::defineCharKinds() {

    tokenMap["{"] = LEFT_BRACKET;
    tokenMap["}"] = RIGHT_BRACKET;
    tokenMap["("] = LEFT_BRACE;
    tokenMap[")"] = RIGHT_BRACE;
    tokenMap[";"] = SEMICOLON;
    tokenMap[":"] = COLON;
    tokenMap[","] = COMMA;
    tokenMap["="] = EQUALS;
    tokenMap["*"] = TIMES;
    tokenMap["**"] = EXPONENT;
    tokenMap["^"] = EXPONENT;
    tokenMap["/"] = DIVIDE;
    tokenMap["+"] = PLUS;
    tokenMap["-"] = MINUS;
    tokenMap["\""] = QUOTE;
    tokenMap["'"] = QUOTE;
    tokenMap["="] = EQUALS;

    tokenMap["=="] = COMPARATIVE_EQUALS;
    tokenMap["!="] = COMPARATIVE_NOT_EQUALS;
    tokenMap[">"] = BIGGER_THAN;
    tokenMap[">="] = BIGGER_OR_EQUAL;
    tokenMap["<"] = SMALLER_THAN;
    tokenMap["<="] = SMALLER_OR_EQUAL;
    tokenMap["mod"] = MODULO;
    tokenMap["div"] = DIV;
    tokenMap["||"] = OR;
    tokenMap["or"] = OR;
    tokenMap["and"] = AND;
    tokenMap["&&"] = AND;
    tokenMap["|"] = BITWISE_OR;
    tokenMap["&"] = BITWISE_AND;
    tokenMap["!|"] = BITWISE_XOR;
    tokenMap["xor"] = XOR;
    tokenMap["!||"] = XOR;
    tokenMap["not"] = NOT;
    tokenMap["!"] = NOT;

    tokenMap["\t"] = WHITESPACE;
    tokenMap["\n"] = WHITESPACE;
    tokenMap[" "] = WHITESPACE;
    tokenMap["if"] = IF;
    tokenMap["else"] = ELSE;
    tokenMap["unless"] = UNLESS;
    tokenMap["method"] = METHOD;
    tokenMap["return"] = RETURN;

    tokenMap[""] = END_OF_FEED;

}

Token Lexer::convertToken(const std::string& substring) {

    static std::regex intRegex(R"(-?[0-9]+)");
    static std::regex floatRegex(R"(-?[0-9]*\.[0-9]+)");

    if (tokenMap[substring] != NONE) {
        return {tokenMap[substring], substring};
    }
    if (std::regex_match(substring, intRegex)) {
        return {INTEGER, substring};
    }
    if (std::regex_match(substring, floatRegex)) {
        return {FLOAT, substring};
    }
    return {IDENTIFIER, substring};
}

bool Lexer::hasNextToken() {
    return (reader <= tokens.size());
}

void Lexer::initReader() {
    reader = 0;
}

std::string Lexer::getParseData() {
    return parseData;
}

void Lexer::moveReader(int d) {
    reader += d;
}

TokenList Lexer::getTokenList() {
    return tokens;
}

void Lexer::back() {
    --reader;
}

Token Lexer::peekToken() {
    Token result = nextToken();
    back();
    return result;
}

Token Lexer::currentToken() {
    back();
    return nextToken();
}

void Lexer::savePosition() {
    positions.push(reader);
}

void Lexer::rollPosition() {
    reader = positions.top();
    Lexer::deletePosition();
}

void Lexer::deletePosition() {
    positions.pop();
}

bool Lexer::expectToken(TokenKind kind) {
    savePosition();
    auto token = nextToken();
    if (token.kind == kind) {
        deletePosition();
        return true;
    }
    rollPosition();
    return false;
}