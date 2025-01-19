# Add subdirectories
add_subdirectory(core)

# Create main framework library
add_library(dsp_framework STATIC "")

# Add framework source files
target_sources(dsp_framework
    PRIVATE
        # Framework source files will be added here
)

# Link with core library and other dependencies
target_link_libraries(dsp_framework
    PRIVATE
        dsp_core
)

# Set include directories
target_include_directories(dsp_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Apply common optimization flags
target_compile_options(dsp_framework
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-mavx2>
)