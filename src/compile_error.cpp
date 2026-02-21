#include "compile_error.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "macros.h"

namespace {

std::vector<std::string> splitLines(const std::string& source) {
    std::vector<std::string> lines;
    std::stringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    if (!source.empty() && source.back() == '\n') {
        lines.push_back("");
    }
    return lines;
}

}

void CompileError::fail(
    const std::string& filePath,
    const std::string& source,
    const Token& token,
    const std::string& message,
    const std::vector<std::string>& expected
) {
    const int line = token.line > 0 ? token.line : 1;
    const int col = token.pos >= 0 ? token.pos + 1 : 1;

    std::cerr << COMPILE_ERROR_PREFIX << filePath << ":" << line << ":" << col << RESET << "\n";
    std::cerr << ERROR_PREFIX << message << RESET << "\n";
    if (!expected.empty()) {
        std::cerr << BOLD_ERROR_PREFIX << "Expected: " << RESET;
        for (size_t i = 0; i < expected.size(); ++i) {
            std::cerr << expected[i];
            if (i + 1 < expected.size()) std::cerr << ", ";
        }
        std::cerr << "\n";
    }

    auto lines = splitLines(source);
    if (lines.empty()) lines.push_back("");

    int from = std::max(1, line - 2);
    int to = std::min((int)lines.size(), line + 2);
    int width = (int)std::to_string(to).size();

    for (int i = from; i <= to; ++i) {
        std::cerr << std::string(width - (int)std::to_string(i).size(), ' ') << i << " | " << lines[i - 1] << "\n";
        if (i == line) {
            int safeCol = std::max(1, col);
            int caretLen = std::max(1, (int)token.value.size());
            std::cerr << std::string(width, ' ') << " | " << std::string(safeCol - 1, ' ') << std::string(caretLen, '^') << "\n";
        }
    }

    std::exit(1);
}
