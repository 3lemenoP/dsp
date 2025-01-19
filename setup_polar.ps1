# setup_polar.ps1
# Creates and populates the polar directory structure

# Ensure we're in the right directory
$rootDir = Get-Location
if (-not (Test-Path "src")) {
    Write-Error "Must be run from dsp_framework root directory"
    exit 1
}

# Create directory structure
$dirs = @(
    "src/polar",
    "src/polar/include/dsp/polar",
    "src/polar/src",
    "src/polar/tests/unit",
    "src/polar/tests/integration"
)

foreach ($dir in $dirs) {
    New-Item -Path $dir -ItemType Directory -Force
    Write-Host "Created directory: $dir"
}

# Create CMakeLists.txt
$cmakeContent = @'
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
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /arch:AVX2>
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
'@

Set-Content -Path "src/polar/CMakeLists.txt" -Value $cmakeContent
Write-Host "Created CMakeLists.txt"

# Create types.hpp
$typesContent = @'
#ifndef DSP_POLAR_TYPES_HPP
#define DSP_POLAR_TYPES_HPP

#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace dsp::polar {

// Forward declarations
template<typename T> class PolarValue;
template<typename T> struct PolarTraits;

/**
 * Traits class for polar value computations.
 */
template<typename T>
struct PolarTraits {
    static_assert(std::is_floating_point_v<T>, "PolarTraits requires floating-point type");
    
    static constexpr T PI = T(3.141592653589793238462643383279502884L);
    static constexpr T TWO_PI = T(2.0) * PI;
    static constexpr T PI_2 = PI / T(2.0);
    static constexpr T EPSILON = std::numeric_limits<T>::epsilon() * T(10);
    static constexpr T MIN_MAGNITUDE = std::numeric_limits<T>::min() * T(2.0);
    static constexpr T PHASE_WRAP_THRESHOLD = PI * T(0.9);
};

class PolarError : public std::runtime_error {
public:
    explicit PolarError(const char* msg) : std::runtime_error(msg) {}
};

template<typename T>
class PolarValue {
    static_assert(std::is_floating_point_v<T>, "PolarValue requires floating-point type");

public:
    PolarValue() noexcept : magnitude_(T(0)), phase_(T(0)) {}
    
    PolarValue(T mag, T phase) {
        setMagnitude(mag);
        setPhase(phase);
    }
    
    T getMagnitude() const noexcept { return magnitude_; }
    T getPhase() const noexcept { return phase_; }
    
    void setMagnitude(T mag) {
        if (mag < T(0)) {
            throw PolarError("Negative magnitude");
        }
        magnitude_ = mag;
    }
    
    void setPhase(T phase) {
        phase_ = normalizePhase(phase);
    }

private:
    T magnitude_;
    T phase_;
    
    static T normalizePhase(T phase) noexcept {
        phase = std::fmod(phase, PolarTraits<T>::TWO_PI);
        if (phase > PolarTraits<T>::PI) {
            phase -= PolarTraits<T>::TWO_PI;
        } else if (phase <= -PolarTraits<T>::PI) {
            phase += PolarTraits<T>::TWO_PI;
        }
        return phase;
    }
};

using PolarFloat = PolarValue<float>;
using PolarDouble = PolarValue<double>;

} // namespace dsp::polar

#endif // DSP_POLAR_TYPES_HPP
'@

Set-Content -Path "src/polar/include/dsp/polar/types.hpp" -Value $typesContent
Write-Host "Created types.hpp"

# Create accumulator.hpp
$accumulatorContent = @'
#ifndef DSP_POLAR_ACCUMULATOR_HPP
#define DSP_POLAR_ACCUMULATOR_HPP

namespace dsp::polar {

template<typename T>
class PhaseAccumulator {
public:
    struct Config {
        T unwrapThreshold;
        bool maintainAbsolute;
        size_t historyLength;
    };

    // Placeholder - implementation to follow
};

} // namespace dsp::polar

#endif // DSP_POLAR_ACCUMULATOR_HPP
'@

Set-Content -Path "src/polar/include/dsp/polar/accumulator.hpp" -Value $accumulatorContent
Write-Host "Created accumulator.hpp"

# Create operations.hpp
$operationsContent = @'
#ifndef DSP_POLAR_OPERATIONS_HPP
#define DSP_POLAR_OPERATIONS_HPP

namespace dsp::polar {

// Placeholder - implementation to follow

} // namespace dsp::polar

#endif // DSP_POLAR_OPERATIONS_HPP
'@

Set-Content -Path "src/polar/include/dsp/polar/operations.hpp" -Value $operationsContent
Write-Host "Created operations.hpp"

# Create accumulator.cpp
$accumulatorImplContent = @'
#include "dsp/polar/accumulator.hpp"

namespace dsp::polar {
// Placeholder - implementation to follow
} // namespace dsp::polar
'@

Set-Content -Path "src/polar/src/accumulator.cpp" -Value $accumulatorImplContent
Write-Host "Created accumulator.cpp"

# Create operations.cpp
$operationsImplContent = @'
#include "dsp/polar/operations.hpp"

namespace dsp::polar {
// Placeholder - implementation to follow
} // namespace dsp::polar
'@

Set-Content -Path "src/polar/src/operations.cpp" -Value $operationsImplContent
Write-Host "Created operations.cpp"

# Create test files
$testFiles = @{
    "src/polar/tests/unit/types_test.cpp" = "// Unit tests for types.hpp - implementation to follow"
    "src/polar/tests/unit/accumulator_test.cpp" = "// Unit tests for accumulator.hpp - implementation to follow"
    "src/polar/tests/unit/operations_test.cpp" = "// Unit tests for operations.hpp - implementation to follow"
    "src/polar/tests/integration/polar_pipeline_test.cpp" = "// Integration tests - implementation to follow"
}

foreach ($file in $testFiles.Keys) {
    Set-Content -Path $file -Value $testFiles[$file]
    Write-Host "Created test file: $file"
}

Write-Host "`nPolar directory structure setup complete!"