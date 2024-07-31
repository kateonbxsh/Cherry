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
using unique = unique_ptr<T>;

#define create_reference make_shared
#define create_unique make_unique