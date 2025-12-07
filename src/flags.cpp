#include "flags.h"

FlagParser::FlagParser(int argc, char** argv) {
    parse(argc, argv);
}

void FlagParser::parse(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--", 0) == 0) {
            // It's a flag
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                // Next argument is a value for this flag
                flags[arg] = argv[i + 1];
                ++i;
            } else {
                // No value, just a flag
                flags[arg] = std::nullopt;
            }
        } else if (arg.rfind("-", 0) == 0) {
            // Short flag, e.g. -d
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                flags[arg] = argv[i + 1];
                ++i;
            } else {
                flags[arg] = std::nullopt;
            }
        } else {
            // Positional argument (likely the file name)
            positionalArgs.push_back(arg);
        }
    }
}

bool FlagParser::hasFlag(const std::string& flag) const {
    return flags.find(flag) != flags.end();
}

std::optional<std::string> FlagParser::getFlagArg(const std::string& flag) const {
    auto it = flags.find(flag);
    if (it != flags.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string FlagParser::getFileName() const {
    if (!positionalArgs.empty()) {
        return positionalArgs.front(); // or .back() for last
    }
    return "";
}

std::vector<std::string> FlagParser::getPositionalArgs() const {
    return positionalArgs;
}