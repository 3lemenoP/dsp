# src/utils/CMakeLists.txt

add_library(dsp_utils OBJECT
    math/math_utils.cpp
    optimization/optimization.cpp
    debug/debug.cpp
)

target_include_directories(dsp_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Create placeholder source files
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/math/math_utils.cpp" "// Math utilities implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/optimization/optimization.cpp" "// Optimization utilities implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/debug/debug.cpp" "// Debug utilities implementation\n")