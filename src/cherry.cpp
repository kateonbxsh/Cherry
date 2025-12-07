#include <iostream>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "flags.h"

using namespace std;
bool __debug_mode = false;

int main(int argCount, char ** args) {

    FlagParser flags(argCount, args);

    if (flags.hasFlag("--help") || flags.hasFlag("-h")) {
        cout << "Usage: cherry <file-path> [--debug] [--output <file>]" << endl;
        return 0;
    }
    if (flags.hasFlag("--debug") || flags.hasFlag("-d")) {
        __debug_mode = true;
        cout << CHERRY_PREFIX << "Debug mode enabled." << endl;
    }

    string fileName = flags.getFileName();
    if (fileName.empty()) {
        cout << "Not enough arguments. Usage: <file-path> <...args>" << endl;
        return 1;
    }

    std::ifstream fileToParse(fileName);
    if (!fileToParse.is_open()) {
        cout << "Failed to open file." << endl;
        return 2;
    }
    string fileContent((std::istreambuf_iterator<char>(fileToParse)), std::istreambuf_iterator<char>());
    fileToParse.close();
    Lexer lexer(fileContent);
    Parser parser(lexer); 
    auto block = parser.parse();
    if (block != nullptr) {
        cout << "Interpreting.." << endl;
        Interpreter interpreter;
        interpreter.interpret(block);
        return 0;
    } else {
        cout << "Block is null" << endl;
        return 2;
    }
    return 1;   
}
