# src/polar/tests/CMakeLists.txt

# Create test executable
add_executable(polar_tests
    unit/types_test.cpp
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

# Add compiler warnings and exception handling
if(MSVC)
    target_compile_options(polar_tests
        PRIVATE
            /W4 
            /EHsc
    )
else()
    target_compile_options(polar_tests
        PRIVATE
            -Wall 
            -Wextra
    )
endif()

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(polar_tests
    DISCOVERY_TIMEOUT 60
)