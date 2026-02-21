#include "runtime_exception.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include "expressions.h"
#include "runtime_builtins.h"

namespace {

std::string gRuntimeFilePath;
std::string gRuntimeSource;
struct RuntimeCallFrame {
    std::string name;
    std::string file;
    int line = -1;
    int col = -1;
};
std::vector<RuntimeCallFrame> gCallFrames;

std::vector<std::string> splitLines(const std::string& source) {
    std::vector<std::string> lines;
    std::stringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) lines.push_back(line);
    if (!source.empty() && source.back() == '\n') lines.push_back("");
    return lines;
}

std::string joinStack(const std::vector<RuntimeCallFrame>& frames) {
    if (frames.empty()) return "<empty>";
    std::string out;
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& frame = frames[frames.size() - i - 1];
        const auto file = frame.file.empty() ? gRuntimeFilePath : frame.file;
        out += "in " + (frame.name.empty() ? std::string("<anonymous>") : frame.name) + "(";
        out += (file.empty() ? std::string("<unknown file>") : file);
        out += ":";
        out += (frame.line >= 1 ? std::to_string(frame.line) : std::string("?"));
        out += ":";
        out += (frame.col >= 1 ? std::to_string(frame.col) : std::string("?"));
        out += ")";
        if (i + 1 < frames.size()) out += "\n";
    }
    return out;
}

Value fieldOrNull(const ClassInstance& inst, const std::string& name) {
    if (inst.fieldValues.contains(name)) return inst.fieldValues.at(name);
    return NullValue;
}

}

void runtimeSetSourceContext(const std::string& filePath, const std::string& source) {
    if (!filePath.empty()) {
        try {
            gRuntimeFilePath = std::filesystem::path(filePath).filename().string();
        } catch (...) {
            gRuntimeFilePath = filePath;
        }
    } else {
        gRuntimeFilePath.clear();
    }
    gRuntimeSource = source;
}

void runtimePushFrame(const std::string& frameName, int line, int col) {
    RuntimeCallFrame frame;
    frame.name = frameName.empty() ? "<anonymous>" : frameName;
    frame.file = gRuntimeFilePath;
    frame.line = line;
    frame.col = col;
    gCallFrames.push_back(frame);
}

void runtimePopFrame() {
    if (!gCallFrames.empty()) gCallFrames.pop_back();
}

reference<Value> makeExceptionRef(const std::string& typeName, const std::string& message, int line, int col) {
    auto typeRef = getExceptionTypeByName(typeName);
    if (typeRef == nullptr) typeRef = getExceptionTypeByName("RuntimeException");
    if (typeRef == nullptr) {
        typeRef = getExceptionTypeBuiltin();
    }
    if (typeRef == nullptr) {
        return create_reference<Value>(Value(message));
    }

    ClassInstance instance;
    instance.classType = typeRef;
    instance.fieldValues["message"] = Value(message);
    instance.fieldValues["stacktrace"] = Value(joinStack(gCallFrames));
    if (line >= 1) instance.fieldValues["line"] = Value((integer)line);
    if (col >= 1) instance.fieldValues["col"] = Value((integer)col);
    if (!gRuntimeFilePath.empty()) instance.fieldValues["file"] = Value(gRuntimeFilePath);

    Value exceptionValue;
    exceptionValue.type = typeRef;
    exceptionValue.value = instance;
    return create_reference<Value>(exceptionValue);
}

Value makeThrown(const std::string& typeName, const std::string& message, int line, int col) {
    Value out;
    out.thrownException = makeExceptionRef(typeName, message, line, col);
    return out;
}

bool isExceptionInstance(const Value& value) {
    auto exceptionType = getExceptionTypeBuiltin();
    if (exceptionType == nullptr) return false;
    return exceptionType->assignableFrom(value);
}

reference<Value> normalizeExceptionRef(const reference<Value>& ex) {
    if (ex == nullptr) {
        return makeExceptionRef("RuntimeException", "unknown exception");
    }
    if (isExceptionInstance(*ex)) return ex;
    if (ex->type == StringType) {
        return makeExceptionRef("Exception", get<string>(ex->value));
    }
    return makeExceptionRef("ValueException", stringify(*ex));
}

void printRuntimeException(const reference<Value>& ex) {
    auto normalized = normalizeExceptionRef(ex);
    std::cerr << EXCEPTION_THROWN_PREFIX;
    if (normalized == nullptr || normalized->type == nullptr || normalized->type->kind != TypeKind::Class) {
        std::cerr << " RuntimeException " << RESET << " " << BOLD_ERROR_PREFIX << "unknown exception" << RESET << "\n";
        return;
    }

    auto inst = get<ClassInstance>(normalized->value);
    auto typeName = inst.classType ? inst.classType->getName() : std::string("Exception");
    auto messageValue = fieldOrNull(inst, "message");
    auto stackValue = fieldOrNull(inst, "stacktrace");
    auto lineValue = fieldOrNull(inst, "line");
    auto colValue = fieldOrNull(inst, "col");
    auto fileValue = fieldOrNull(inst, "file");

    std::string message = stringify(messageValue);
    std::string file = fileValue.type == StringType ? get<string>(fileValue.value) : gRuntimeFilePath;
    int line = lineValue.type == IntegerType ? (int)get<integer>(lineValue.value) : -1;
    int col = colValue.type == IntegerType ? (int)get<integer>(colValue.value) : -1;

    std::cerr << " " << typeName << " " << RESET << " " << BOLD_ERROR_PREFIX << message << ERROR_PREFIX << "\n";
    std::cerr << "at " << (file.empty() ? std::string("<unknown file>") : file) << ":" << RESET;
    std::cerr << (line >= 1 ? std::to_string(line) : std::string("?")) << ":";
    std::cerr << (col >= 1 ? std::to_string(col) : std::string("?")) << "\n";

    if (!gRuntimeSource.empty() && line >= 1) {
        auto lines = splitLines(gRuntimeSource);
        int from = std::max(1, line - 2);
        int to = std::min((int)lines.size(), line + 2);
        int width = (int)std::to_string(to).size();
        for (int i = from; i <= to; ++i) {
            std::cerr << std::string(width - (int)std::to_string(i).size(), ' ') << i << " | " << lines[i - 1] << "\n";
            if (i == line) {
                int safeCol = std::max(1, col);
                std::cerr << std::string(width, ' ') << " | " << std::string(safeCol - 1, ' ') << "^\n";
            }
        }
    }
    std::string stack = stackValue.type == StringType ? get<string>(stackValue.value) : "<empty>";
    std::cerr << ERROR_PREFIX;
    if (stack == "<empty>") {
        std::cerr << "in global scope\n";
    } else {
        std::cerr << stack << "\n";
    }
}
