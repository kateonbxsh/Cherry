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

#define RESET "\e[0m"
// Cherry prefix: pink background, bold white text
#define CHERRY_PREFIX "\e[45m\e[1;37m Cherry \e[0m "

// Debug prefix: cherry prefix + DEBUG, cyan background, bold white text
#define DEBUG_PREFIX "\e[46m\e[1;37m DEBUG \e[0m "

// Debug error prefix: cherry + debug, red text
#define DEBUG_ERROR_PREFIX DEBUG_PREFIX "\e[0;31m"

// Debug success prefix: cherry + debug, green text
#define DEBUG_SUCCESS_PREFIX DEBUG_PREFIX "\e[0;32m"

// Debug warning prefix: cherry + debug, yellow text
#define DEBUG_WARNING_PREFIX DEBUG_PREFIX "\e[0;33m"

// Compile error prefix: cherry + compile error, magenta background, bold white text
#define COMPILE_ERROR_PREFIX "\e[41m\e[1;37m COMPILE ERROR \e[0m\e[1;31m "

// Exception thrown prefix: cherry + exception thrown, yellow background, bold white text
#define EXCEPTION_THROWN_PREFIX "\e[41m\e[1;37m EXCEPTION THROWN \e[0m\e[1;31m "
#define ERROR_PREFIX "\e[0;31m"
#define BOLD_ERROR_PREFIX "\e[1;31m"

extern bool __debug_mode;
#define DEBUG __debug_mode