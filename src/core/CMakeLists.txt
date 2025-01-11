# src/core/CMakeLists.txt

add_library(dsp_core OBJECT
    fft/fft.cpp
    filters/filter.cpp
    modulation/modulator.cpp
    pll/pll.cpp
)

target_include_directories(dsp_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Add compiler flags specific to DSP core
target_compile_options(dsp_core
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-mavx2>
)

# Create placeholder source files
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/fft/fft.cpp" "// FFT implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/filters/filter.cpp" "// Filter implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/modulation/modulator.cpp" "// Modulator implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/pll/pll.cpp" "// PLL implementation\n")