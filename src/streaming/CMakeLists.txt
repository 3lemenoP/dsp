# src/streaming/CMakeLists.txt

add_library(dsp_streaming OBJECT
    input_handler/input_handler.cpp
    output_handler/output_handler.cpp
    pipeline/pipeline.cpp
)

target_include_directories(dsp_streaming
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Create placeholder source files
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/input_handler/input_handler.cpp" "// Input handler implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/output_handler/output_handler.cpp" "// Output handler implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/pipeline.cpp" "// Pipeline implementation\n")