# Create core library
add_library(dsp_core OBJECT
    fft/fft_plan.cpp
    fft/fft.cpp                     # Placeholder file
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

# Create placeholder source files if they don't exist
set(PLACEHOLDER_FILES
    fft/fft.cpp
    filters/filter.cpp
    modulation/modulator.cpp
    pll/pll.cpp
)

foreach(FILE ${PLACEHOLDER_FILES})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
        # For fft.cpp, add a note about fft_plan implementation
        if(${FILE} STREQUAL "fft/fft.cpp")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}"
                "// FFT implementation placeholder\n"
                "// Main FFT functionality is currently implemented in fft_plan.hpp/cpp\n"
            )
        else()
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" "// ${FILE} implementation\n")
        endif()
    endif()
endforeach()