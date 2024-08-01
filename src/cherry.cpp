#include <iostream>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#define DEBUG_LEXER_TOKEN false

using namespace std;

int main(int argCount, char ** args) {
    if (argCount < 2) {
        cout << "Not enough arguments. Usage: <file-path> <...args>" << endl;
        return 1;
    }
    std::ifstream fileToParse (args[1]);
    if (!fileToParse.is_open()) {
        cout << "Failed to open file." << endl;
        return 2;
    }
    string fileContent((std::istreambuf_iterator<char>(fileToParse)), std::istreambuf_iterator<char>());
    fileToParse.close();
    Lexer lexer(fileContent);
    #if DEBUG_LEXER_TOKEN 
        for(auto token : lexer.getTokenList()) {
            std::cout << token.value << ", " << std::endl;
        }
    #endif
    Parser parser(lexer);
    auto block = parser.parse();
    if (block != nullptr) {
        cout << "Interpreting.." << endl;
        Interpreter interpreter;
        interpreter.interpret(block);
        return 0;
    }
    return 1;   
}
