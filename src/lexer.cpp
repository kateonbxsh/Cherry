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

        // ---------- COMMENTS ----------
        if (c == '/' && i + 1 < parseData.size()) {
            if (parseData[i + 1] == '/') {
                i += 2;
                pos += 2;
                while (i < parseData.size() && parseData[i] != '\n') {
                    i++;
                    pos++;
                }
                continue;
            }
            if (parseData[i + 1] == '*') {
                i += 2;
                pos += 2;
                while (i + 1 < parseData.size()) {
                    if (parseData[i] == '\n') {
                        i++;
                        line++;
                        pos = 0;
                        continue;
                    }
                    if (parseData[i] == '*' && parseData[i + 1] == '/') {
                        i += 2;
                        pos += 2;
                        break;
                    }
                    i++;
                    pos++;
                }
                continue;
            }
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
        if (std::isalpha(c) || c == '_') {
            std::string value;
            while (i < parseData.size()) {
                char ch = parseData[i];
                if (std::isalnum(ch) || ch == '_') {
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

        // try longest match first (3-char operators)
        if (i + 2 < parseData.size()) {
            std::string three = parseData.substr(i, 3);
            auto it3 = tokenMap.find(three);
            if (it3 != tokenMap.end()) {
                pushToken(it3->second, three, startPos, startLine);
                i += 3;
                pos += 3;
                continue;
            }
        }

        // then 2-char operators
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

        std::string one(1, c);
        auto it1 = tokenMap.find(one);
        if (it1 != tokenMap.end()) {
            pushToken(it1->second, one, startPos, startLine);
            i++;
            pos++;
            continue;
        }

        i++;
        pos++;
    }

    pushToken(END_OF_FEED, "", pos, line);
}


Token Lexer::nextToken() {
    if (reader < tokens.size()) {
        return tokens.at(reader++);
    }
    const int fallbackLine = tokens.empty() ? 1 : tokens.back().line + 1;
    reader = tokens.size();
    return {END_OF_FEED, "", 0, fallbackLine};
}

void Lexer::defineCharKinds() {

    tokenMap["["] = LEFT_BRACKET;
    tokenMap["]"] = RIGHT_BRACKET;
    tokenMap["{"] = LEFT_BRACE;
    tokenMap["}"] = RIGHT_BRACE;
    tokenMap["("] = LEFT_PARENTHESIS;
    tokenMap[")"] = RIGHT_PARENTHESIS;
    tokenMap["=>"] = ARROW;
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
    tokenMap["is"] = IS;
    tokenMap["||"] = OR;
    tokenMap["or"] = OR;
    tokenMap["and"] = AND;
    tokenMap["&&"] = AND;
    tokenMap["|"] = BITWISE_OR;
    tokenMap["&"] = BITWISE_AND;
    tokenMap["!|"] = BITWISE_XOR;
    tokenMap["xor"] = XOR;
    tokenMap["^||"] = XOR;
    tokenMap["not"] = NOT;
    tokenMap["!"] = NOT;

    tokenMap["\t"] = WHITESPACE;
    tokenMap["\n"] = WHITESPACE;
    tokenMap[" "] = WHITESPACE;
    tokenMap["let"] = INFER;
    tokenMap["infer"] = INFER;
    tokenMap["if"] = IF;
    tokenMap["else"] = ELSE;
    tokenMap["unless"] = UNLESS;
    tokenMap["try"] = TRY;
    tokenMap["catch"] = CATCH;
    tokenMap["finally"] = FINALLY;
    tokenMap["throw"] = THROW;
    tokenMap["repeat"] = REPEAT;
    tokenMap["return"] = RETURN;
    tokenMap["null"] = NULL_TOKEN;
    tokenMap["true"] = TRUE;
    tokenMap["false"] = FALSE;
    tokenMap["for"] = FOR;
    tokenMap["while"] = WHILE;
    tokenMap["do"] = DO;
    tokenMap["until"] = UNTIL;
    tokenMap["times"] = REPEAT_TIMES;
    tokenMap["class"] = CLASS;
    tokenMap["type"] = TYPE;
    tokenMap["extends"] = EXTENDS;
    tokenMap["when"] = WHEN;
    tokenMap["default"] = DEFAULT;
    tokenMap["public"] = PUBLIC;
    tokenMap["private"] = PRIVATE;
    tokenMap["protected"] = PROTECTED;
    tokenMap["sealed"] = SEALED;
    tokenMap["static"] = STATIC;
    tokenMap["new"] = NEW;
    tokenMap["."] = DOT;
    tokenMap["..."] = ELLIPSIS;
    tokenMap["this"] = THIS;
    tokenMap["display"] = DISPLAY;

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
    if (reader > 0) {
        --reader;
    }
}

Token Lexer::peekToken() {
    Token result = nextToken();
    back();
    return result;
}

Token Lexer::currentToken() {
    if (reader == 0) {
        return peekToken();
    }
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
    "try",
    "catch",
    "finally",
    "throw",
    "return",
    "for",
    "while",
    "repeat",
    "do",
    "until",
    "times",
    "right_parenthesis",
    "left_parenthesis",
    "right_bracket",
    "left_bracket",
    "right_brace",
    "left_brace",
    "arrow",
    "comma",
    "semicolon",
    "colon",
    "equals",
    "class",
    "type",
    "extends",
    "when",
    "default",
    "public",
    "private",
    "protected",
    "sealed",
    "static",
    "new",
    "dot",
    "ellipsis",
    "this",
    "display",
    
    "begin_of_binary_operators",
    "comparative_equals", "comparative_not_equals",
    "bigger_than", "bigger_or_equal",
    "smaller_than", "smaller_or_equal",
    "times", "exponent",
    "divide",
    "plus",
    "minus",
    "bitwise_and", "bitwise_or", "bitwise_xor",
    "and", "or", "xor", 
    "modulo", "div", "is",
    "end_of_binary_operators",

    "begin_of_unary_operators",
    "not",
    "end_of_unary_operators",

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
