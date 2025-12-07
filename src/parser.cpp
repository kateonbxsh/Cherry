#include <utility>
#include <algorithm>
#include <set>
#include <sstream>

#include "parser.h"
#include "statement.h"
#include "statements/variable/VariableDefinition.h"
#include "statements/GlobalBlock.h"
#include "macros.h"

Parser::Parser(Lexer& inputLexer) {
    lexer = inputLexer;
}

uref<GlobalBlock> Parser::parse() {

    lexer.initReader();

    auto block = GlobalBlock::parse(lexer);

    if (!block->valid) {

        auto lastToken = block->lastToken;
        std::cerr << COMPILE_ERROR_PREFIX << "\"" << lastToken.value << "\" at line " << lastToken.line << ", char " << lastToken.pos << std::endl;
        std::cerr << ERROR_PREFIX << "Unexpected token " << lastToken.value << " of type: " << tokenKindStrings[lastToken.kind] << std::endl;
        if (!block->expected.empty()) {
            std::cerr << BOLD_ERROR_PREFIX << "Expected: " << ERROR_PREFIX;
            for (size_t i = 0; i < block->expected.size(); ++i) {
                std::cerr << block->expected[i];
                if (i != block->expected.size() - 1) std::cerr << ", ";
            }
        }
        std::cerr << RESET << std::endl;
        exit(1);

        return nullptr;

    }

    return block;

}