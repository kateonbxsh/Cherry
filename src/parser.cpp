#include <utility>
#include <algorithm>
#include <set>
#include <sstream>

#include "parser.h"
#include "statement.h"
#include "statements/variable/VariableDefinition.h"
#include "statements/GlobalBlock.h"

Parser::Parser(Lexer& inputLexer) {
    lexer = inputLexer;
}

unique<GlobalBlock> Parser::parse() {

    lexer.initReader();

    auto block = GlobalBlock::parse(lexer);

    if (!block->valid) {

        auto lastToken = block->lastToken;
        std::cerr << "[FATAL] CompileError: \"" << lastToken.value << "\" at line " << lastToken.line << ", char " << lastToken.pos << std::endl;
        std::cerr << "Unexpected token " << lastToken.value << " of type: " << tokenKindStrings[lastToken.kind] << std::endl;
        std::cerr << "Expected: " << block->expected;
        std::cerr << std::endl;

        return nullptr;

    }

    return move(block);

}