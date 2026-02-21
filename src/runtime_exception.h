#pragma once

#include <string>
#include "data.h"

void runtimeSetSourceContext(const std::string& filePath, const std::string& source);
void runtimePushFrame(const std::string& frameName, int line = -1, int col = -1);
void runtimePopFrame();

reference<Value> makeExceptionRef(
    const std::string& typeName,
    const std::string& message,
    int line = -1,
    int col = -1
);

Value makeThrown(
    const std::string& typeName,
    const std::string& message,
    int line = -1,
    int col = -1
);

reference<Value> normalizeExceptionRef(const reference<Value>& ex);
bool isExceptionInstance(const Value& value);
void printRuntimeException(const reference<Value>& ex);
