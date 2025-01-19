cmake_minimum_required(VERSION 3.20)

# Project definition
project(dsp_framework
    VERSION 0.1.0
    DESCRIPTION "High-performance DSP Framework (Optimized for i5-6300U)"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(DSP_ENABLE_BENCHMARKS "Enable benchmark tests" OFF)
option(DSP_ENABLE_EXAMPLES "Build example programs" OFF)
option(BUILD_TESTING "Build the testing tree." ON)
option(DSP_ENABLE_AVX2 "Enable AVX2 optimizations" ON)  # New option for FFT SIMD

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Fetch and configure Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Optionally fetch Google Benchmark
if(DSP_ENABLE_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    set(BENCHMARK_ENABLE_TESTING OFF)
    FetchContent_MakeAvailable(benchmark)
endif()

# Common compiler flags and definitions
set(DSP_COMMON_COMPILE_OPTIONS "")
set(DSP_COMMON_COMPILE_DEFINITIONS
    THREAD_POOL_SIZE=4
    CACHE_LINE_SIZE=64
    L1_CACHE_SIZE=32768
)

# Optimization flags for i5-6300U
if(MSVC)
    # Common MSVC flags
    list(APPEND DSP_COMMON_COMPILE_OPTIONS
        /W4 
        /permissive- 
        /MP4   # Parallel compilation (4 threads)
    )
    
    # Add AVX2 support if enabled
    if(DSP_ENABLE_AVX2)
        list(APPEND DSP_COMMON_COMPILE_OPTIONS /arch:AVX2)
    endif()
    
    # Set runtime library
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND DSP_COMMON_COMPILE_OPTIONS /MDd)
    else()
        list(APPEND DSP_COMMON_COMPILE_OPTIONS /MD)
    endif()
    
    # Configuration-specific flags
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /GL /Gy /O2 /Oi")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /Od /RTC1")
    
    # Link-time optimization (Release only)
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
    # Common GCC/Clang flags
    list(APPEND DSP_COMMON_COMPILE_OPTIONS
        -Wall 
        -Wextra 
        -march=native
    )
    
    # Add AVX2 support if enabled
    if(DSP_ENABLE_AVX2)
        list(APPEND DSP_COMMON_COMPILE_OPTIONS -mavx2)
    endif()
    
    # Configuration-specific flags
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -ffunction-sections -fdata-sections")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0 -g")
endif()

# Function to set common properties for DSP targets
function(dsp_set_target_properties target)
    target_compile_options(${target} PRIVATE ${DSP_COMMON_COMPILE_OPTIONS})
    target_compile_definitions(${target} PRIVATE ${DSP_COMMON_COMPILE_DEFINITIONS})
    
    # Set output directories
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endfunction()

# Enable testing
enable_testing()

# Add source directory
add_subdirectory(src)

# Add tests directory if testing is enabled
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Optionally add examples
if(DSP_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "DSP Framework Configuration:")
message(STATUS "  Version:            ${CMAKE_PROJECT_VERSION}")
message(STATUS "  Build type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "  AVX2 support:       ${DSP_ENABLE_AVX2}")
message(STATUS "  Testing enabled:     ${BUILD_TESTING}")
message(STATUS "  Benchmarks enabled:  ${DSP_ENABLE_BENCHMARKS}")
message(STATUS "  Examples enabled:    ${DSP_ENABLE_EXAMPLES}")
message(STATUS "")