cmake_minimum_required(VERSION 3.26)
project(proglang)

set(CMAKE_CXX_STANDARD 17)

add_executable(proglang main.cpp
        src/lexer.cpp
        src/lexer.h
        src/compiler.h
        src/compiler.cpp
        src/scope.h
        src/data.h
        src/scope.cpp
        src/expression.cpp
        src/type.cpp
        src/type.h
        src/statement.h
        src/statements/VariableDefinition.h
        src/executor.cpp
        src/executor.h
        src/expression.h
)
