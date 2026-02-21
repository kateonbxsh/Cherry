#pragma once

#include <string>
#include <vector>
#include "lexer.h"

namespace CompileError {

void fail(
    const std::string& filePath,
    const std::string& source,
    const Token& token,
    const std::string& message,
    const std::vector<std::string>& expected = {}
);

}
