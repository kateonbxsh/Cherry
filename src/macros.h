#pragma once

#include <variant>
#include <memory>

using namespace std;

typedef double real;
typedef int64_t integer;
typedef char boolean;

template<typename T>
using reference = shared_ptr<T>;

template<typename T>
using uref = unique_ptr<T>;

#define create_reference make_shared
#define create_unique make_unique

#define RESET "\x1b[0m"
// Cherry prefix: pink background, bold white text
#define CHERRY_PREFIX "\x1b[45m\x1b[1;37m Cherry \x1b[0m "

#define DEBUG_TABS __debug_tabs
extern int __debug_tabs;

// Debug prefix: cherry prefix + DEBUG, cyan background, bold white text
#define DEBUG_PREFIX (std::string("\x1b[46m\x1b[1;37m DEBUG \x1b[0m ") + std::string(DEBUG_TABS * 4, ' '))

// Debug error prefix: cherry + debug, red text
#define DEBUG_ERROR_PREFIX DEBUG_PREFIX + "\x1b[0;31m"

// Debug success prefix: cherry + debug, green text
#define DEBUG_SUCCESS_PREFIX DEBUG_PREFIX + "\x1b[0;32m"

// Debug warning prefix: cherry + debug, yellow text
#define DEBUG_WARNING_PREFIX DEBUG_PREFIX + "\x1b[0;33m"

// Compile error prefix: cherry + compile error, magenta background, bold white text
#define COMPILE_ERROR_PREFIX "\x1b[41m\x1b[1;37m COMPILE ERROR \x1b[0m\x1b[1;31m "

// Exception thrown prefix: cherry + exception thrown, yellow background, bold white text
#define EXCEPTION_THROWN_PREFIX "\x1b[41m\x1b[1;37m"
#define ERROR_PREFIX "\x1b[0;31m"
#define BOLD_ERROR_PREFIX "\x1b[1;31m"

extern bool __debug_mode;
#define DEBUG __debug_mode
