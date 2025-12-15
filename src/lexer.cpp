#include <regex>
#include "lexer.h"

Lexer::Lexer() = default;
Lexer::Lexer(std::string data) {

    defineCharKinds();
    parseData = std::move(data);

    size_t i = 0;
    size_t pos = 0;
    size_t line = 1;

    auto pushToken = [&](TokenKind kind, const std::string& value, size_t p, size_t l) {
        tokens.push_back({kind, value, (int)p, (int)l});
    };

    while (i < parseData.size()) {

        char c = parseData[i];

        // ---------- WHITESPACE ----------
        if (c == ' ' || c == '\t' || c == '\r') {
            i++;
            pos++;
            continue;
        }

        if (c == '\n') {
            i++;
            line++;
            pos = 0;
            continue;
        }

        size_t startPos = pos;
        size_t startLine = line;

        // ---------- STRING ----------
        if (c == '"') {
            i++; pos++;
            std::string value;

            while (i < parseData.size() && parseData[i] != '"') {
                if (parseData[i] == '\n') {
                    line++;
                    pos = 0;
                } else {
                    pos++;
                }
                value += parseData[i++];
            }

            // Consume closing quote
            if (i < parseData.size()) {
                i++; pos++;
            }

            pushToken(STRING, value, startPos, startLine);
            continue;
        }

        // ---------- IDENTIFIER / KEYWORD ----------
        if (std::isalpha(c) || c == '_' || c == '-') {
            std::string value;
            while (i < parseData.size()) {
                char ch = parseData[i];
                if (std::isalnum(ch) || ch == '_' || ch == '-') {
                    value += ch;
                    i++; pos++;
                } else {
                    break;
                }
            }

            // Keyword resolution happens HERE, not mid-token
            auto it = tokenMap.find(value);
            if (it != tokenMap.end()) {
                pushToken(it->second, value, startPos, startLine);
            } else {
                pushToken(IDENTIFIER, value, startPos, startLine);
            }
            continue;
        }

        // ---------- NUMBER ----------
        if (std::isdigit(c) || (c == '-' && i + 1 < parseData.size() && std::isdigit(parseData[i + 1]))) {
            std::string value;
            bool isFloat = false;

            if (c == '-') {
                value += c;
                i++; pos++;
            }

            while (i < parseData.size() && std::isdigit(parseData[i])) {
                value += parseData[i++];
                pos++;
            }

            if (i < parseData.size() && parseData[i] == '.') {
                isFloat = true;
                value += '.';
                i++; pos++;

                while (i < parseData.size() && std::isdigit(parseData[i])) {
                    value += parseData[i++];
                    pos++;
                }
            }

            pushToken(isFloat ? FLOAT : INTEGER, value, startPos, startLine);
            continue;
        }

        // ---------- OPERATORS / PUNCTUATION ----------
        // Try longest match first (2-char operators)
        if (i + 1 < parseData.size()) {
            std::string two = parseData.substr(i, 2);
            auto it2 = tokenMap.find(two);
            if (it2 != tokenMap.end()) {
                pushToken(it2->second, two, startPos, startLine);
                i += 2;
                pos += 2;
                continue;
            }
        }

        // Fallback to single character
        std::string one(1, c);
        auto it1 = tokenMap.find(one);
        if (it1 != tokenMap.end()) {
            pushToken(it1->second, one, startPos, startLine);
            i++;
            pos++;
            continue;
        }

        // ---------- UNKNOWN ----------
        // Skip unknown characters safely
        i++;
        pos++;
    }

    pushToken(END_OF_FEED, "", pos, line);
}


Token Lexer::nextToken() {
    if (reader < tokens.size()) {
        return tokens.at(reader++);
    }
    reader++;
    return {END_OF_FEED, "", 0, tokens.at(reader-2).line + 1};
}

void Lexer::defineCharKinds() {

    tokenMap["["] = LEFT_BRACKET;
    tokenMap["]"] = RIGHT_BRACKET;
    tokenMap["{"] = LEFT_BRACE;
    tokenMap["}"] = RIGHT_BRACE;
    tokenMap["("] = LEFT_PARENTHESIS;
    tokenMap[")"] = RIGHT_PARENTHESIS;
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
    tokenMap["infer"] = INFER;
    tokenMap["if"] = IF;
    tokenMap["else"] = ELSE;
    tokenMap["unless"] = UNLESS;
    tokenMap["method"] = METHOD;
    tokenMap["return"] = RETURN;
    tokenMap["null"] = NULL_TOKEN;
    tokenMap["true"] = TRUE;
    tokenMap["false"] = FALSE;

    tokenMap[""] = END_OF_FEED;

}

Token Lexer::convertToken(const std::string& substring) {

    static std::regex intRegex(R"(-?[0-9]+)");
    static std::regex floatRegex(R"(-?(([0-9]*\.[0-9]*)|([0-9]+f)))");

    if (tokenMap[substring] != NONE) {
        return {tokenMap[substring], substring, 0, 0};
    }
    if (std::regex_match(substring, intRegex)) {
        return {INTEGER, substring, 0, 0};
    }
    if (std::regex_match(substring, floatRegex)) {
        return {FLOAT, substring, 0, 0};
    }
    return {IDENTIFIER, substring, 0, 0};
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

const std::vector<std::string> tokenKindStrings = {
    "none",
    "whitespace",
    "string",
    "integer",
    "float",
    "null_token",
    "true",
    "false",
    "identifier",
    "infer",
    "if",
    "unless",
    "else",
    "function",
    "method",
    "return",
    "right_parenthesis",
    "left_parenthesis",
    "right_bracket",
    "left_bracket",
    "right_brace",
    "left_brace",
    "comma",
    "semicolon",
    "colon",
    "equals",
    "begin_of_operators",
    "comparative_equals", "comparative_not_equals",
    "bigger_than", "bigger_or_equal",
    "smaller_than", "smaller_or_equal",
    "times", "exponent",
    "divide",
    "plus",
    "minus",
    "bitwise_and", "bitwise_or", "bitwise_xor",
    "and", "or", "xor", "not",
    "modulo", "div",
    "end_of_operators",
    "quote",
    "end_of_feed"
};

std::vector<std::string> tokenKindsToString(const std::vector<TokenKind>& tokenKinds) {
    std::vector<std::string> result;
    for (const auto& token : tokenKinds) {
        if (token >= 0 && token < tokenKindStrings.size()) {
            result.push_back(tokenKindStrings[token]);
        } else {
            result.push_back("unknown");
        }
    }
    return result;
}