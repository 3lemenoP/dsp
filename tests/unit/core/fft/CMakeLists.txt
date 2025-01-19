# FFT test configuration
add_executable(fft_plan_test
    fft_plan_test.cpp
)

target_link_libraries(fft_plan_test
    PRIVATE
        dsp_core
        gtest
        gtest_main
)

# Set include directories
target_include_directories(fft_plan_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Add compiler flags specific to DSP core tests
target_compile_options(fft_plan_test
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-mavx2>
)

# Add test to CTest
add_test(NAME FFTPlanTest COMMAND fft_plan_test)

# Set up test properties
set_tests_properties(FFTPlanTest PROPERTIES
    LABELS "unit;core;fft"
)