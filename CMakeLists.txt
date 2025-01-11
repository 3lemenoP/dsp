cmake_minimum_required(VERSION 3.20)

# Project definition
project(dsp_framework
    VERSION 0.1.0
    DESCRIPTION "High-performance DSP Framework (Optimized for i5-6300U)"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Optimization flags for i5-6300U
if(MSVC)
    # Common MSVC flags
    add_compile_options(
        /W4 
        /permissive- 
        /arch:AVX2
        /MP4   # Parallel compilation (4 threads)
    )
    
    # Set runtime library
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/MDd)
    else()
        add_compile_options(/MD)
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
    add_compile_options(-Wall -Wextra -march=native -mavx2)
    
    # Configuration-specific flags
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -ffunction-sections -fdata-sections")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0 -g")
endif()

# Thread pool size (optimized for 4 logical processors)
add_definitions(-DTHREAD_POOL_SIZE=4)
# Cache line size (64 bytes for this CPU)
add_definitions(-DCACHE_LINE_SIZE=64)
# L1 cache size (32KB per core)
add_definitions(-DL1_CACHE_SIZE=32768)

# Enable testing first
enable_testing()

# Then add the source directory
add_subdirectory(src)

# Set up tests configuration
add_subdirectory(tests)