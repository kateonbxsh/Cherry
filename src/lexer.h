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
    METHOD,
    RETURN,
    RIGHT_BRACKET,
    LEFT_BRACKET,
    RIGHT_BRACE,
    LEFT_BRACE,
    COMMA,
    SEMICOLON,
    COLON,
    EQUALS,

    BEGIN_OF_OPERATORS,
    COMPARATIVE_EQUALS, COMPARATIVE_NOT_EQUALS,
    BIGGER_THAN, BIGGER_OR_EQUAL,
    SMALLER_THAN, SMALLER_OR_EQUAL,
    TIMES, EXPONENT,
    DIVIDE,
    PLUS,
    MINUS,
    BITWISE_AND, BITWISE_OR, BITWISE_XOR,
    AND, OR, XOR, NOT,
    MODULO, DIV,
    END_OF_OPERATORS,

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
    int reader = 0;
};

extern const std::vector<std::string> tokenKindStrings;

std::vector<std::string> tokenKindsToString(const std::vector<TokenKind>& tokenKinds);
