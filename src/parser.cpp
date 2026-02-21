#include <utility>
#include <algorithm>
#include <set>
#include <sstream>

#include "parser.h"
#include "compile_error.h"
#include "statement.h"
#include "statements/variable/VariableDefinition.h"
#include "statements/GlobalBlock.h"
#include "macros.h"

Parser::Parser(Lexer& inputLexer, std::string filePath) : filePath(std::move(filePath)) {
    lexer = inputLexer;
}

uref<GlobalBlock> Parser::parse() {

    lexer.initReader();

    auto block = GlobalBlock::parse(lexer);

    if (!block->valid) {
        auto lastToken = block->lastToken;
        std::string message = "Unexpected token '" + lastToken.value + "' of type " + tokenKindStrings[lastToken.kind];
        if (!block->errorMessage.empty()) {
            message = block->errorMessage;
        }
        CompileError::fail(filePath, lexer.getParseData(), lastToken, message, block->expected);
    }

    return block;

}
