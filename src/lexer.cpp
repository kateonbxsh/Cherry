#include <regex>
#include "lexer.h"


Lexer::Lexer() = default;
Lexer::Lexer(std::string data) {

    defineCharKinds();

    parseData = data;
    std::string currentReadValue;
    int i, pos = 0, line = 1, currentPos, currentLine;
    bool inString = false; std::string currentString;
    for(i = 0; i < data.length(); (i++, pos++)) {
        std::string currentChar = std::string(1, data[i]);
        TokenKind kindOfChar = tokenMap[currentChar];
        if (!inString && currentChar == "\"") {
            inString = true;
            currentString = "";
            continue;
        } else if (inString && currentChar == "\"") {
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
        if (kindOfChar == NONE) {
            if (currentReadValue.empty()) {currentLine = line; currentPos = pos;}
            currentReadValue.append(currentChar);
            continue;
        }
        if (!currentReadValue.empty()) {
            Token result = convertToken(currentReadValue);
            result.line = currentLine; result.pos = currentPos;
            tokens.push_back(result);
            currentReadValue.clear();
        }
        if (kindOfChar != WHITESPACE) {
            Token result = convertToken(currentChar);
            result.line = line; result.pos = pos;
            tokens.push_back(result);
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
    tokenMap["/"] = DIVIDE;
    tokenMap["+"] = PLUS;
    tokenMap["-"] = MINUS;
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
    return {NAME, substring};
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
