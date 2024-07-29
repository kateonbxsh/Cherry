#include <iostream>
#include <fstream>
#include <parser.h>
#include <interpreter.h>

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
    std::string fileContent((std::istreambuf_iterator<char>(fileToParse)), std::istreambuf_iterator<char>());
    fileToParse.close();
    Lexer lexer(fileContent);
    Parser parser(lexer);
    auto block = parser.parse();
    if (block != nullptr) {
        Interpreter interpreter;
        interpreter.interpret(block);
        return 0;
    }
    return 1;   
}
