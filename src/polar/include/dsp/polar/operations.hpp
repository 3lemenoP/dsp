#ifndef DSP_POLAR_OPERATIONS_HPP
#define DSP_POLAR_OPERATIONS_HPP

#include "types.hpp"
#include <complex>
#include <iostream>

namespace dsp::polar {

/**
 * @brief Rectangular form representation optimized for SIMD operations
 * 
 * PERFORMANCE: Fields ordered for optimal memory alignment
 */
template<typename T>
struct RectangularForm {
    static_assert(std::is_floating_point_v<T>, "RectangularForm requires floating-point type");
    alignas(2 * sizeof(T)) T real;
    T imag;
};

/**
 * @brief Core operations for polar values
 * 
 * PERFORMANCE: Uses SIMD operations where available
 * THREAD-SAFETY: All operations are thread-safe
 */
template<typename T>
class PolarOperations {
public:
    // Debug helper functions - can be conditionally compiled out
#ifdef DSP_DEBUG
    static void debugPrint(const char* msg, const RectangularForm<T>& rect, T threshold = T(0)) {
        std::cout << msg << ": real=" << rect.real
            << ", imag=" << rect.imag
            << ", magnitude=" << std::hypot(rect.real, rect.imag);
        if (threshold != T(0)) {
            std::cout << ", threshold=" << threshold;
        }
        std::cout << std::endl;
    }

    static void debugPrint(const char* msg, const PolarValue<T>& polar) {
        std::cout << msg << ": magnitude=" << polar.getMagnitude()
            << ", phase=" << polar.getPhase()
            << " (" << (polar.getPhase() * 180.0 / PolarTraits<T>::PI) << " degrees)"
            << std::endl;
    }
#else
    static void debugPrint(const char*, const RectangularForm<T>&, T = T(0)) {}
    static void debugPrint(const char*, const PolarValue<T>&) {}
#endif

    /**
     * @brief Check if rectangular coordinates are effectively zero
     * 
     * PERFORMANCE: Uses SIMD-friendly comparison operations
     */
    static bool isEffectivelyZeroRect(const RectangularForm<T>& rect, T scale = T(1)) noexcept {
        const T threshold = PolarTraits<T>::MIN_MAGNITUDE * scale;
        return std::abs(rect.real) <= threshold && std::abs(rect.imag) <= threshold;
    }

    /**
     * @brief Determine zero threshold based on operand magnitudes
     * 
     * PERFORMANCE: Uses vectorizable max operation
     */
    static T determineZeroThreshold(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
        T maxMag = std::max(a.getMagnitude(), b.getMagnitude());
        return maxMag * PolarTraits<T>::EPSILON * T(100);
    }

    /**
     * @brief Multiply two polar values
     * 
     * PERFORMANCE: Optimized for zero checks and phase normalization
     */
    static PolarValue<T> multiply(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
        if (a.isZero() || b.isZero()) {
            return PolarValue<T>();
        }
        return PolarValue<T>(
            a.magnitude_ * b.magnitude_,
            normalizePhase(a.phase_ + b.phase_)
        );
    }

    /**
     * @brief Divide two polar values
     * 
     * PERFORMANCE: Optimized for phase computation
     */
    static PolarValue<T> divide(const PolarValue<T>& a, const PolarValue<T>& b) {
        if (b.isZero()) {
            throw PolarError("Division by zero magnitude");
        }
        return PolarValue<T>(
            a.magnitude_ / b.magnitude_,
            normalizePhase(a.phase_ - b.phase_)
        );
    }

    /**
     * @brief Add two polar values
     * 
     * PERFORMANCE: Uses optimized rectangular conversion and SIMD operations
     */
    static PolarValue<T> add(const PolarValue<T>& a, const PolarValue<T>& b) {
        debugPrint("Input a", a);
        debugPrint("Input b", b);

        if (a.isZero()) return b;
        if (b.isZero()) return a;

        auto rectA = toRectangular(a);
        auto rectB = toRectangular(b);
        
        RectangularForm<T> sum{
            rectA.real + rectB.real,
            rectA.imag + rectB.imag
        };

        T threshold = determineZeroThreshold(a, b);
        if (isEffectivelyZeroRect(sum, threshold)) {
            return PolarValue<T>();
        }

        return toPolar(sum);
    }

    /**
     * @brief Subtract two polar values
     * 
     * PERFORMANCE: Uses optimized rectangular conversion and SIMD operations
     */
    static PolarValue<T> subtract(const PolarValue<T>& a, const PolarValue<T>& b) {
        debugPrint("Input a", a);
        debugPrint("Input b", b);

        if (a.isZero()) return scale(b, T(-1));
        if (b.isZero()) return a;

        auto rectA = toRectangular(a);
        auto rectB = toRectangular(b);

        RectangularForm<T> diff{
            rectA.real - rectB.real,
            rectA.imag - rectB.imag
        };

        T threshold = determineZeroThreshold(a, b);
        if (isEffectivelyZeroRect(diff, threshold)) {
            return PolarValue<T>();
        }

        return toPolar(diff);
    }

    /**
     * @brief Scale a polar value
     * 
     * PERFORMANCE: Optimized phase handling for negative scalars
     */
    static PolarValue<T> scale(const PolarValue<T>& value, T scalar) {
        debugPrint("Input", value);

        if (PolarValue<T>::isEffectivelyZero(scalar)) {
            return PolarValue<T>();
        }

        if (scalar < T(0)) {
            return PolarValue<T>(
                value.magnitude_ * -scalar,
                normalizePhase(value.phase_ + PolarTraits<T>::PI)
            );
        }

        return PolarValue<T>(value.magnitude_ * scalar, value.phase_);
    }

    /**
     * @brief Compute complex conjugate
     */
    static PolarValue<T> conjugate(const PolarValue<T>& value) noexcept {
        return PolarValue<T>(value.magnitude_, -value.phase_);
    }

    /**
     * @brief Compute multiplicative inverse
     */
    static PolarValue<T> reciprocal(const PolarValue<T>& value) {
        if (value.isZero()) {
            throw PolarError("Reciprocal of zero");
        }
        return PolarValue<T>(T(1) / value.magnitude_, -value.phase_);
    }

    /**
     * @brief Multiply by conjugate
     * 
     * PERFORMANCE: Optimized phase computation
     */
    static PolarValue<T> multiplyConjugate(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
        return PolarValue<T>(
            a.magnitude_ * b.magnitude_,
            normalizePhase(a.phase_ - b.phase_)
        );
    }

    /**
     * @brief Rotate by angle
     */
    static PolarValue<T> rotate(const PolarValue<T>& value, T angle) noexcept {
        debugPrint("Input", value);
        return PolarValue<T>(
            value.magnitude_,
            normalizePhase(value.phase_ + angle)
        );
    }

    /**
     * @brief Convert from rectangular to polar coordinates
     * 
     * PERFORMANCE: Uses optimized hypot and atan2 implementations
     */
    static PolarValue<T> toPolar(const RectangularForm<T>& rect) {
        if (isEffectivelyZeroRect(rect)) {
            return PolarValue<T>();
        }

        T magnitude = std::hypot(rect.real, rect.imag);
        T phase = std::atan2(rect.imag, rect.real);

        return PolarValue<T>(magnitude, phase);
    }

    /**
     * @brief Convert from polar to rectangular coordinates
     * 
     * PERFORMANCE: Uses SIMD-optimized sincos where available
     */
    static RectangularForm<T> toRectangular(const PolarValue<T>& polar) {
        if (polar.isZero()) {
            return RectangularForm<T>{T(0), T(0)};
        }

        RectangularForm<T> result;
#if defined(__GNUC__) || defined(__clang__)
        T sinVal, cosVal;
        __builtin_sincos(polar.phase_, &sinVal, &cosVal);
        result.real = polar.magnitude_ * cosVal;
        result.imag = polar.magnitude_ * sinVal;
#else
        result.real = polar.magnitude_ * std::cos(polar.phase_);
        result.imag = polar.magnitude_ * std::sin(polar.phase_);
#endif

        return result;
    }

private:
    static T normalizePhase(T phase) noexcept {
        return dsp::polar::normalizePhase(phase);
    }
};

// Operator overloads
template<typename T>
PolarValue<T> operator*(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
    return PolarOperations<T>::multiply(a, b);
}

template<typename T>
PolarValue<T> operator/(const PolarValue<T>& a, const PolarValue<T>& b) {
    return PolarOperations<T>::divide(a, b);
}

template<typename T>
PolarValue<T> operator+(const PolarValue<T>& a, const PolarValue<T>& b) {
    return PolarOperations<T>::add(a, b);
}

template<typename T>
PolarValue<T> operator-(const PolarValue<T>& a, const PolarValue<T>& b) {
    return PolarOperations<T>::subtract(a, b);
}

template<typename T>
PolarValue<T> operator*(const PolarValue<T>& value, T scalar) {
    return PolarOperations<T>::scale(value, scalar);
}

template<typename T>
PolarValue<T> operator*(T scalar, const PolarValue<T>& value) {
    return PolarOperations<T>::scale(value, scalar);
}

// Convenience functions
template<typename T>
PolarValue<T> toPolar(const RectangularForm<T>& rect) {
    return PolarOperations<T>::toPolar(rect);
}

template<typename T>
RectangularForm<T> toRectangular(const PolarValue<T>& polar) {
    return PolarOperations<T>::toRectangular(polar);
}

} // namespace dsp::polar

#endif // DSP_POLAR_OPERATIONS_HPP