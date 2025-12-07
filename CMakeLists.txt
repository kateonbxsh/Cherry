cmake_minimum_required(VERSION 3.22)
project(Cherry)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Source files
set(SOURCE_FILES
    src/types/types.cpp
    src/cherry.cpp
    src/expressions.cpp
    src/interpreter.cpp
    src/lexer.cpp
    src/parser.cpp
    src/scope.cpp
    src/data.cpp
    src/flags.cpp
    src/statement.cpp
    src/statements/GlobalBlock.cpp
    src/statements/GeneralStatement.cpp
    src/statements/variable/VariableDefinition.cpp
    src/statements/variable/VariableAffectation.cpp
    src/statements/expression/Expression.cpp
)

add_executable(Cherry ${SOURCE_FILES})