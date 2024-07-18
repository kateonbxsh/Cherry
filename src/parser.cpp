#include <utility>
#include <algorithm>
#include <set>
#include <sstream>

#include "parser.h"
#include "statement.h"
#include "statements/definition/VariableDefinition.h"
#include "statements/GlobalBlock.h"

Parser::Parser(Lexer& inputLexer) {
    lexer = inputLexer;
}

Statement* Parser::parse() {

    lexer.initReader();

    auto block = GlobalBlock::parse(lexer);

    if (!block->valid) {

        auto lastToken = block->lastToken;
        std::cerr << "CompileError: \"" << lastToken.value << "\" at line " << lastToken.line << ", char " << lastToken.pos << std::endl;
        std::cerr << "Unexpected token " << lastToken.value << " of type " << lastToken.kind << std::endl;
        std::cerr << "Expected: ";
        for (auto expected : block->expected) {
            std::cerr << expected << " ";
        }
        std::cerr << std::endl;

        delete block;
        return nullptr;

    }

    return block;

}