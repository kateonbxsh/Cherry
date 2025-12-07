#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>

class FlagParser {
public:
    FlagParser(int argc, char** argv);

    // Returns true if the flag (e.g. "--debug") is present (no argument)
    bool hasFlag(const std::string& flag) const;

    // Returns the argument for a flag (e.g. "--output out.txt"), or std::nullopt if not present
    std::optional<std::string> getFlagArg(const std::string& flag) const;

    // Returns the file name (first or last non-flag argument)
    std::string getFileName() const;

    // Returns all non-flag arguments (excluding flags and their values)
    std::vector<std::string> getPositionalArgs() const;

private:
    std::unordered_map<std::string, std::optional<std::string>> flags;
    std::vector<std::string> positionalArgs;
    void parse(int argc, char** argv);
};