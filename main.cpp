#include <iostream>
#include <fstream>
#include "src/compiler.h"
#include "src/executor.h"

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
    Compiler compiler(lexer);
    Block block = compiler.parse();
    Value result = Executor::execute(block);
    if (result.type == "int") {
        return any_cast<int>(result.value);
    }
    return 0;   
}
