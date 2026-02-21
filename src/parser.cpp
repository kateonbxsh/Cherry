#include <utility>
#include <algorithm>
#include <set>
#include <sstream>
#include <filesystem>

#include "parser.h"
#include "compile_error.h"
#include "statement.h"
#include "statements/variable/VariableDefinition.h"
#include "statements/GlobalBlock.h"
#include "macros.h"
#include "runtime_exception.h"

Parser::Parser(Lexer& inputLexer, std::string filePath) : filePath(std::move(filePath)) {
    lexer = inputLexer;
}

uref<GlobalBlock> Parser::parse() {

    lexer.initReader();
    std::string absolutePath = filePath;
    try {
        absolutePath = std::filesystem::absolute(filePath).string();
    } catch (...) {}
    runtimeSetSourceContext(absolutePath, lexer.getParseData());

    auto block = GlobalBlock::parse(lexer);

    if (!block->valid) {
        auto lastToken = block->lastToken;
        std::string message = "Unexpected token '" + lastToken.value + "' of type " + tokenKindStrings[lastToken.kind];
        if (!block->errorMessage.empty()) {
            message = block->errorMessage;
        }
        CompileError::fail(absolutePath, lexer.getParseData(), lastToken, message, block->expected);
    }

    return block;

}
