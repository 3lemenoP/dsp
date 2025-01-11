# src/analysis/CMakeLists.txt

add_library(dsp_analysis OBJECT
    spectrum/spectrum.cpp
    modulation/modulation.cpp
    quality/quality.cpp
)

target_include_directories(dsp_analysis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Create placeholder source files
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/spectrum/spectrum.cpp" "// Spectrum analysis implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/modulation/modulation.cpp" "// Modulation analysis implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/quality/quality.cpp" "// Quality analysis implementation\n")