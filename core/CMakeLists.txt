add_library(calc_core
    src/lexer.cpp
    src/parser.cpp
    src/session.cpp
)

target_include_directories(calc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(calc_core PRIVATE -Wall -Wextra -Wpedantic)
