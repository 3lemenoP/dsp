# Create test executable
add_executable(polar_tests
    unit/types_test.cpp
    unit/operations_test.cpp
)

# Configure test executable
target_link_libraries(polar_tests
    PRIVATE
        dsp_polar
        GTest::gtest
        GTest::gtest_main
)

# Include directories
target_include_directories(polar_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/polar/include
        ${CMAKE_SOURCE_DIR}/src
)

# Set C++ standard for tests
set_target_properties(polar_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Add compiler warnings and optimization flags
if(MSVC)
    target_compile_options(polar_tests
        PRIVATE
            /W4 
            /arch:AVX2
            /EHsc
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:Debug>:/Od /Zi>
    )
else()
    target_compile_options(polar_tests
        PRIVATE
            -Wall 
            -Wextra
            -march=native
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Debug>:-O0 -g>
    )
endif()

# Optional: Add benchmark tests if enabled
if(DSP_ENABLE_BENCHMARKS)
    add_executable(polar_benchmarks
        benchmarks/operations_benchmark.cpp
    )
    
    target_link_libraries(polar_benchmarks
        PRIVATE
            dsp_polar
            benchmark::benchmark
    )
    
    set_target_properties(polar_benchmarks PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(polar_benchmarks
            PRIVATE
                /W4 
                /arch:AVX2
                /EHsc
                $<$<CONFIG:Release>:/O2>
        )
    else()
        target_compile_options(polar_benchmarks
            PRIVATE
                -Wall 
                -Wextra
                -march=native
                $<$<CONFIG:Release>:-O3>
        )
    endif()
endif()

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(polar_tests
    DISCOVERY_TIMEOUT 60
)