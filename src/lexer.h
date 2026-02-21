#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <stack>

enum TokenKind {

    NONE,
    WHITESPACE,
    STRING,
    INTEGER,
    FLOAT,
    NULL_TOKEN,
    TRUE,
    FALSE,
    IDENTIFIER,
    INFER,
    IF,
    UNLESS,
    ELSE,
    RETURN,
    FOR,
    WHILE,
    REPEAT,
    DO,
    UNTIL,
    REPEAT_TIMES,
    RIGHT_PARENTHESIS,
    LEFT_PARENTHESIS,
    RIGHT_BRACKET,
    LEFT_BRACKET,
    RIGHT_BRACE,
    LEFT_BRACE,
    ARROW,
    COMMA,
    SEMICOLON,
    COLON,
    EQUALS,

    CLASS,
    EXTENDS,
    PUBLIC,
    PRIVATE,
    PROTECTED,
    SEALED,
    STATIC,

    BEGIN_OF_BINARY_OPERATORS,
    COMPARATIVE_EQUALS, COMPARATIVE_NOT_EQUALS,
    BIGGER_THAN, BIGGER_OR_EQUAL,
    SMALLER_THAN, SMALLER_OR_EQUAL,
    TIMES, EXPONENT,
    DIVIDE,
    PLUS,
    MINUS,
    BITWISE_AND, BITWISE_OR, BITWISE_XOR,
    AND, OR, XOR,
    MODULO, DIV,
    END_OF_BINARY_OPERATORS,

    BEGIN_OF_UNARY_OPERATORS,
    NOT,
    END_OF_UNARY_OPERATORS,

    QUOTE,
    END_OF_FEED
};


struct Token {
    TokenKind kind;
    std::string value;
    int pos;
    int line;
};

typedef std::vector<Token> TokenList;

class Lexer{
public:
    Lexer();
    explicit Lexer(std::string data);

    Token nextToken();
    Token peekToken();
    Token currentToken();
    bool expectToken(TokenKind kind);
    void savePosition();
    void rollPosition();
    void deletePosition();
    void back();
    bool hasNextToken();
    void initReader();
    void moveReader(int d);
    std::string getParseData();
    TokenList getTokenList();

private:
    void defineCharKinds();

    Token convertToken(const std::string& substring);

    std::string parseData;
    std::map<std::string, TokenKind> tokenMap;
    std::vector<Token> tokens;
    std::stack<int> positions;
    long long unsigned int reader = 0;
};

extern const std::vector<std::string> tokenKindStrings;

std::vector<std::string> tokenKindsToString(const std::vector<TokenKind>& tokenKinds);
