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

    template<typename T>
    struct PolarTraits {
        static_assert(std::is_floating_point_v<T>, "PolarTraits requires floating-point type");

        // Core constants
        static constexpr T PI = T(3.14159265358979323846);
        static constexpr T TWO_PI = T(2) * PI;

        // Numerical thresholds
        // - Increase these multipliers to allow more "slop" near boundaries
        static constexpr T EPSILON = std::numeric_limits<T>::epsilon() * T(10);
        static constexpr T MIN_MAGNITUDE = std::numeric_limits<T>::min() * T(2);
        static constexpr T MAX_MAGNITUDE = std::numeric_limits<T>::max() / T(2);

        // Phase thresholds
        // - We go quite high on the boundary epsilon to handle (3*PI -> -PI) etc.
        static constexpr T PHASE_EPSILON = EPSILON * T(10);
        static constexpr T PI_BOUNDARY_EPSILON = PHASE_EPSILON * T(20);
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

        // Accessors
        T getMagnitude() const noexcept { return magnitude_; }
        T getPhase() const noexcept { return phase_; }

        // Mutators
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

        // In-place polar multiplication
        PolarValue& operator*=(const PolarValue& rhs) noexcept {
            if (isEffectivelyZero(magnitude_) || isEffectivelyZero(rhs.magnitude_)) {
                magnitude_ = T(0);
                phase_ = T(0);
                return *this;
            }
            magnitude_ *= rhs.magnitude_;
            phase_ = normalizePhase(phase_ + rhs.phase_);
            return *this;
        }

        // In-place polar division
        PolarValue& operator/=(const PolarValue& rhs) {
            if (isEffectivelyZero(rhs.magnitude_)) {
                throw PolarError("Division by zero magnitude");
            }
            magnitude_ /= rhs.magnitude_;
            phase_ = normalizePhase(phase_ - rhs.phase_);
            return *this;
        }

        // In-place scalar multiplication
        PolarValue& operator*=(T scalar) {
            if (isEffectivelyZero(scalar)) {
                magnitude_ = T(0);
                phase_ = T(0);
                return *this;
            }
            if (scalar < T(0)) {
                magnitude_ *= -scalar;
                // Shift phase by +π for negative scalar
                phase_ = normalizePhase(phase_ + PolarTraits<T>::PI);
            }
            else {
                magnitude_ *= scalar;
            }
            return *this;
        }

        // Comparisons
        bool operator==(const PolarValue& rhs) const noexcept {
            // Both effectively zero -> treat as equal
            if (isEffectivelyZero(magnitude_) && isEffectivelyZero(rhs.magnitude_)) {
                return true;
            }
            // Compare magnitudes with relative threshold
            T maxMag = (magnitude_ > rhs.magnitude_) ? magnitude_ : rhs.magnitude_;
            T magDiff = std::abs(magnitude_ - rhs.magnitude_);
            if (magDiff > PolarTraits<T>::EPSILON * maxMag) {
                return false;
            }
            // Compare phases
            return isPhaseEqual(phase_, rhs.phase_);
        }

        bool operator!=(const PolarValue& rhs) const noexcept {
            return !(*this == rhs);
        }

        // Utility
        bool isZero() const noexcept {
            return isEffectivelyZero(magnitude_);
        }

        PolarValue conjugate() const noexcept {
            return PolarValue(magnitude_, -phase_);
        }

        PolarValue reciprocal() const {
            if (isZero()) {
                throw PolarError("Reciprocal of zero");
            }
            return PolarValue(T(1) / magnitude_, -phase_);
        }

    private:
        T magnitude_;
        T phase_;

        // Check if value is "close enough" to zero magnitude
        static bool isEffectivelyZero(T val) noexcept {
            return (std::abs(val) <= PolarTraits<T>::MIN_MAGNITUDE);
        }

        static bool isPhaseEqual(T pA, T pB) noexcept {
            // Normalized difference
            T diff = normalizePhase(pA - pB);
            // If difference is near zero or near ±2π
            return (std::abs(diff) <= PolarTraits<T>::PHASE_EPSILON) ||
                (std::abs(std::abs(diff) - PolarTraits<T>::TWO_PI) <= PolarTraits<T>::PHASE_EPSILON);
        }

        // Snap angles near 0 or ±π
        static void snapToBoundary(T& angle) noexcept {
            // Snap near zero
            if (std::abs(angle) < PolarTraits<T>::PI_BOUNDARY_EPSILON) {
                angle = T(0);
                return;
            }
            // Snap near +π
            T diffPosPi = std::abs(angle - PolarTraits<T>::PI);
            if (diffPosPi < PolarTraits<T>::PI_BOUNDARY_EPSILON) {
                angle = PolarTraits<T>::PI;
                return;
            }
            // Snap near -π
            T diffNegPi = std::abs(angle + PolarTraits<T>::PI);
            if (diffNegPi < PolarTraits<T>::PI_BOUNDARY_EPSILON) {
                angle = -PolarTraits<T>::PI;
                return;
            }
        }

        static T normalizePhase(T raw) noexcept {
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
            snapToBoundary(raw);
            return raw;
        }
    };

    // Non-member operators
    template<typename T>
    PolarValue<T> operator*(PolarValue<T> lhs, const PolarValue<T>& rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    template<typename T>
    PolarValue<T> operator/(PolarValue<T> lhs, const PolarValue<T>& rhs) {
        lhs /= rhs;
        return lhs;
    }

    template<typename T>
    PolarValue<T> operator*(PolarValue<T> value, T scalar) {
        value *= scalar;
        return value;
    }

    template<typename T>
    PolarValue<T> operator*(T scalar, PolarValue<T> value) {
        value *= scalar;
        return value;
    }

    // Common aliases
    using PolarFloat = PolarValue<float>;
    using PolarDouble = PolarValue<double>;

} // namespace dsp::polar

#endif // DSP_POLAR_TYPES_HPP
