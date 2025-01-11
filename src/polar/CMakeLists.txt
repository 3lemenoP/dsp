# File: src/polar/CMakeLists.txt

# Define the polar library as an OBJECT library for better optimization
add_library(dsp_polar OBJECT)

# Set target properties
set_target_properties(dsp_polar PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Source files
target_sources(dsp_polar
    PRIVATE
        include/dsp/polar/types.hpp
        include/dsp/polar/accumulator.hpp
        include/dsp/polar/operations.hpp
        src/accumulator.cpp
        src/operations.cpp
)

# Include directories
target_include_directories(dsp_polar
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler options
target_compile_options(dsp_polar
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /arch:AVX2 /EHsc>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -march=native>
)

# Dependencies
target_link_libraries(dsp_polar
    PUBLIC
        dsp_core
        dsp_utils
)

# Installation rules
install(TARGETS dsp_polar
    EXPORT dsp_polarTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/dsp
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Add tests subdirectory
add_subdirectory(tests)