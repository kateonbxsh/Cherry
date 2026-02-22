cmake_minimum_required(VERSION 3.22)
project(Cherry)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Source files
set(SOURCE_FILES
    src/types/type.cpp
    src/types/function.cpp
    src/types/types.cpp
    src/cherry.cpp
    src/expressions.cpp
    src/interpreter.cpp
    src/lexer.cpp
    src/parser.cpp
    src/type_syntax.cpp
    src/scope.cpp
    src/data.cpp
    src/runtime_exception.cpp
    src/flags.cpp
    src/compile_error.cpp
    src/runtime_builtins.cpp
    src/statement.cpp
    src/statements/Block.cpp
    src/statements/GlobalBlock.cpp
    src/statements/GeneralStatement.cpp
    src/statements/variable/VariableDefinition.cpp
    src/statements/variable/VariableAffectation.cpp
    src/statements/expression/Expression.cpp
    src/statements/function/LambdaExpression.cpp
    src/statements/function/FunctionCall.cpp
    src/statements/function/ExpressionStatement.cpp
    src/statements/function/ReturnStatement.cpp
    src/statements/exception/ThrowStatement.cpp
    src/statements/exception/TryCatchFinallyStatement.cpp
    src/statements/conditional/IfStatement.cpp
    src/statements/loop/LoopStatement.cpp
    src/statements/class/Class.cpp
    src/statements/type/TypeDefinition.cpp
)

add_executable(Cherry ${SOURCE_FILES})

if(EMSCRIPTEN)
    target_link_options(Cherry PRIVATE
        -sFORCE_FILESYSTEM=1
        -sINVOKE_RUN=0
        -sEXIT_RUNTIME=0
        -sEXPORTED_RUNTIME_METHODS=['callMain','FS']
    )
endif()
