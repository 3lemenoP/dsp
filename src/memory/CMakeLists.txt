# src/memory/CMakeLists.txt

add_library(dsp_memory OBJECT
    circular_buffer/circular_buffer.cpp
    simd_allocator/simd_allocator.cpp
    cache_manager/cache_manager.cpp
)

target_include_directories(dsp_memory
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Create placeholder source files
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/circular_buffer/circular_buffer.cpp" "// Circular buffer implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/simd_allocator/simd_allocator.cpp" "// SIMD allocator implementation\n")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/cache_manager/cache_manager.cpp" "// Cache manager implementation\n")