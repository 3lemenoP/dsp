# src/CMakeLists.txt

# Core library components
add_subdirectory(core)
# Memory management components
add_subdirectory(memory)
# Streaming components
add_subdirectory(streaming)
# Analysis components
add_subdirectory(analysis)
# Utility components
add_subdirectory(utils)
# Polar components
add_subdirectory(polar)

# Create main DSP library
add_library(dsp_framework STATIC
    $<TARGET_OBJECTS:dsp_core>
    $<TARGET_OBJECTS:dsp_memory>
    $<TARGET_OBJECTS:dsp_streaming>
    $<TARGET_OBJECTS:dsp_analysis>
    $<TARGET_OBJECTS:dsp_utils>
    $<TARGET_OBJECTS:dsp_polar>
)

target_include_directories(dsp_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Set library properties
set_target_properties(dsp_framework PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

# Export the target
install(TARGETS dsp_framework
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)