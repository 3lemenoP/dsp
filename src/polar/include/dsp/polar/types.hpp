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
 * @brief Traits class for polar coordinate system calculations
 * @tparam T Floating point type for calculations
 * 
 * PERFORMANCE: Constants are constexpr for compile-time optimization
 * MEMORY: All members are static to avoid instance overhead
 */
template<typename T>
struct PolarTraits {
    static_assert(std::is_floating_point_v<T>, "PolarTraits requires floating-point type");

    // Core constants
    static constexpr T PI = T(3.14159265358979323846);
    static constexpr T TWO_PI = T(2) * PI;

    // Numerical thresholds - tuned for optimal balance of precision and performance
    static constexpr T EPSILON = std::numeric_limits<T>::epsilon() * T(10);
    static constexpr T MIN_MAGNITUDE = std::numeric_limits<T>::min() * T(2);
    static constexpr T MAX_MAGNITUDE = std::numeric_limits<T>::max() / T(2);
    static constexpr T PHASE_EPSILON = EPSILON * T(10);
    static constexpr T PI_BOUNDARY_EPSILON = PHASE_EPSILON * T(20);
};

/**
 * @brief Exception class for polar coordinate operations
 */
class PolarError : public std::runtime_error {
public:
    explicit PolarError(const char* msg) : std::runtime_error(msg) {}
};

/**
 * @brief Namespace-level phase normalization functions
 * PERFORMANCE: These are marked constexpr where possible for compile-time evaluation
 */
namespace detail {
    template<typename T>
    constexpr void snapToBoundary(T& angle) noexcept {
        // PERFORMANCE: Early exit for common case
        if (std::abs(angle) >= PolarTraits<T>::PI_BOUNDARY_EPSILON &&
            std::abs(std::abs(angle) - PolarTraits<T>::PI) >= PolarTraits<T>::PI_BOUNDARY_EPSILON) {
            return;
        }

        // Snap near zero
        if (std::abs(angle) < PolarTraits<T>::PI_BOUNDARY_EPSILON) {
            angle = T(0);
            return;
        }

        // Snap near ±π
        if (std::abs(angle - PolarTraits<T>::PI) < PolarTraits<T>::PI_BOUNDARY_EPSILON) {
            angle = PolarTraits<T>::PI;
        }
        else if (std::abs(angle + PolarTraits<T>::PI) < PolarTraits<T>::PI_BOUNDARY_EPSILON) {
            angle = -PolarTraits<T>::PI;
        }
    }
} // namespace detail

/**
 * @brief Normalize phase angle to [-π, π] range
 * @tparam T Floating point type
 * @param raw Input phase angle
 * @return Normalized phase angle
 * 
 * PERFORMANCE: Uses fmod for efficient range reduction
 * THREAD-SAFETY: Thread-safe due to no shared state
 */
template<typename T>
constexpr T normalizePhase(T raw) noexcept {
    // Bring into [-2π, 2π]
    raw = std::fmod(raw, PolarTraits<T>::TWO_PI);

    // Map to [-π, π]
    if (raw > PolarTraits<T>::PI) {
        raw -= PolarTraits<T>::TWO_PI;
    }
    else if (raw <= -PolarTraits<T>::PI) {
        raw += PolarTraits<T>::TWO_PI;
    }

    // Snap to exact ±π or 0 if close enough
    detail::snapToBoundary(raw);
    return raw;
}

/**
 * @brief Class representing a value in polar coordinates
 * @tparam T Floating point type for storage
 * 
 * PERFORMANCE: Optimized for SIMD operations through data layout
 * THREAD-SAFETY: Thread-safe for const operations
 */
template<typename T>
class PolarValue {
    static_assert(std::is_floating_point_v<T>, "PolarValue requires floating-point type");

public:
    // Constructors
    constexpr PolarValue() noexcept : magnitude_(T(0)), phase_(T(0)) {}

    PolarValue(T mag, T phase) {
        setMagnitude(mag);
        setPhase(phase);
    }

    // Core accessors
    constexpr T getMagnitude() const noexcept { return magnitude_; }
    constexpr T getPhase() const noexcept { return phase_; }

    // Core mutators
    void setMagnitude(T mag) {
        if (mag < T(0)) {
            throw PolarError("Negative magnitude");
        }
        if (mag > PolarTraits<T>::MAX_MAGNITUDE) {
            throw PolarError("Magnitude exceeds maximum safe value");
        }
        magnitude_ = mag;
    }

    void setPhase(T phase) {
        phase_ = normalizePhase(phase);
    }

    // State queries
    constexpr bool isZero() const noexcept {
        return isEffectivelyZero(magnitude_);
    }

    // Comparison operators
    bool operator==(const PolarValue& rhs) const noexcept {
        // Both effectively zero -> treat as equal
        if (isEffectivelyZero(magnitude_) && isEffectivelyZero(rhs.magnitude_)) {
            return true;
        }

        // Compare magnitudes with relative threshold
        T maxMag = std::max(magnitude_, rhs.magnitude_);
        T magDiff = std::abs(magnitude_ - rhs.magnitude_);
        if (magDiff > PolarTraits<T>::EPSILON * maxMag) {
            return false;
        }

        // Compare phases for non-zero magnitudes
        return isPhaseEqual(phase_, rhs.phase_);
    }

    constexpr bool operator!=(const PolarValue& rhs) const noexcept {
        return !(*this == rhs);
    }

private:
    // MEMORY: Aligned for SIMD operations
    alignas(2 * sizeof(T)) T magnitude_;
    T phase_;

    // Helper functions
    static constexpr bool isEffectivelyZero(T val) noexcept {
        return (std::abs(val) <= PolarTraits<T>::MIN_MAGNITUDE);
    }

    static bool isPhaseEqual(T pA, T pB) noexcept {
        // Normalized difference
        T diff = normalizePhase(pA - pB);
        // Check if difference is near zero or near ±2π
        return (std::abs(diff) <= PolarTraits<T>::PHASE_EPSILON) ||
               (std::abs(std::abs(diff) - PolarTraits<T>::TWO_PI) <= PolarTraits<T>::PHASE_EPSILON);
    }

    // Friend declaration for operations
    template<typename U>
    friend class PolarOperations;
};

// Common type aliases
using PolarFloat = PolarValue<float>;
using PolarDouble = PolarValue<double>;

} // namespace dsp::polar

#endif // DSP_POLAR_TYPES_HPP