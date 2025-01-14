#ifndef DSP_POLAR_OPERATIONS_HPP
#define DSP_POLAR_OPERATIONS_HPP

#include "types.hpp"
#include <complex>
#include <iostream>

namespace dsp::polar {

    /**
     * @brief Rectangular form representation
     */
    template<typename T>
    struct RectangularForm {
        T real;
        T imag;
        static_assert(std::is_floating_point_v<T>, "RectangularForm requires floating-point type");
    };

    /**
     * @brief Core operations for polar values
     */
    template<typename T>
    class PolarOperations {
    public:
        // Debug helper functions
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

        // Core helper functions
        static bool isEffectivelyZeroRect(const RectangularForm<T>& rect, T scale = T(1)) noexcept {
            const T threshold = PolarTraits<T>::MIN_MAGNITUDE * scale;
            bool result = std::abs(rect.real) <= threshold && std::abs(rect.imag) <= threshold;

            std::cout << "Zero check: |real|=" << std::abs(rect.real)
                << ", |imag|=" << std::abs(rect.imag)
                << ", threshold=" << threshold
                << ", result=" << (result ? "true" : "false") << std::endl;

            return result;
        }

        static T determineZeroThreshold(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
            T maxMag = std::max(a.getMagnitude(), b.getMagnitude());
            T threshold = maxMag * PolarTraits<T>::EPSILON * T(100); // Increased factor for stability

            std::cout << "Zero threshold calculation:"
                << "\n  a.magnitude=" << a.getMagnitude()
                << "\n  b.magnitude=" << b.getMagnitude()
                << "\n  maxMag=" << maxMag
                << "\n  EPSILON=" << PolarTraits<T>::EPSILON
                << "\n  threshold=" << threshold << std::endl;

            return threshold;
        }

        // Basic operations
        static PolarValue<T> multiply(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
            if (a.isZero() || b.isZero()) {
                return PolarValue<T>();
            }
            return PolarValue<T>(
                a.magnitude_ * b.magnitude_,
                normalizePhase(a.phase_ + b.phase_)
            );
        }

        static PolarValue<T> divide(const PolarValue<T>& a, const PolarValue<T>& b) {
            if (b.isZero()) {
                throw PolarError("Division by zero magnitude");
            }
            return PolarValue<T>(
                a.magnitude_ / b.magnitude_,
                normalizePhase(a.phase_ - b.phase_)
            );
        }

        static PolarValue<T> add(const PolarValue<T>& a, const PolarValue<T>& b) {
            std::cout << "\nAddition Debug:" << std::endl;
            debugPrint("Input a", a);
            debugPrint("Input b", b);

            if (a.isZero()) return b;
            if (b.isZero()) return a;

            auto rectA = toRectangular(a);
            auto rectB = toRectangular(b);
            debugPrint("rectA", rectA);
            debugPrint("rectB", rectB);

            RectangularForm<T> sum{
                rectA.real + rectB.real,
                rectA.imag + rectB.imag
            };
            debugPrint("sum", sum);

            T threshold = determineZeroThreshold(a, b);
            if (isEffectivelyZeroRect(sum, threshold)) {
                std::cout << "Sum detected as zero" << std::endl;
                return PolarValue<T>();
            }

            auto result = toPolar(sum);
            debugPrint("Result", result);
            return result;
        }

        static PolarValue<T> subtract(const PolarValue<T>& a, const PolarValue<T>& b) {
            std::cout << "\nSubtraction Debug:" << std::endl;
            debugPrint("Input a", a);
            debugPrint("Input b", b);

            if (a.isZero()) return scale(b, T(-1));
            if (b.isZero()) return a;

            auto rectA = toRectangular(a);
            auto rectB = toRectangular(b);
            debugPrint("rectA", rectA);
            debugPrint("rectB", rectB);

            RectangularForm<T> diff{
                rectA.real - rectB.real,
                rectA.imag - rectB.imag
            };
            debugPrint("diff", diff);

            T threshold = determineZeroThreshold(a, b);
            if (isEffectivelyZeroRect(diff, threshold)) {
                std::cout << "Difference detected as zero" << std::endl;
                return PolarValue<T>();
            }

            auto result = toPolar(diff);
            debugPrint("Result", result);
            return result;
        }

        static PolarValue<T> scale(const PolarValue<T>& value, T scalar) {
            std::cout << "\nScaling Debug:" << std::endl;
            debugPrint("Input", value);
            std::cout << "Scalar: " << scalar << std::endl;

            if (PolarValue<T>::isEffectivelyZero(scalar)) {
                return PolarValue<T>();
            }

            if (scalar < T(0)) {
                auto result = PolarValue<T>(
                    value.magnitude_ * -scalar,
                    normalizePhase(value.phase_ + PolarTraits<T>::PI)
                );
                debugPrint("Result (negative scalar)", result);
                return result;
            }

            auto result = PolarValue<T>(value.magnitude_ * scalar, value.phase_);
            debugPrint("Result (positive scalar)", result);
            return result;
        }

        static PolarValue<T> conjugate(const PolarValue<T>& value) noexcept {
            return PolarValue<T>(value.magnitude_, -value.phase_);
        }

        static PolarValue<T> reciprocal(const PolarValue<T>& value) {
            if (value.isZero()) {
                throw PolarError("Reciprocal of zero");
            }
            return PolarValue<T>(T(1) / value.magnitude_, -value.phase_);
        }

        static PolarValue<T> multiplyConjugate(const PolarValue<T>& a, const PolarValue<T>& b) noexcept {
            return PolarValue<T>(
                a.magnitude_ * b.magnitude_,
                normalizePhase(a.phase_ - b.phase_)
            );
        }

        static PolarValue<T> rotate(const PolarValue<T>& value, T angle) noexcept {
            std::cout << "\nRotation Debug:" << std::endl;
            debugPrint("Input", value);
            std::cout << "Rotation angle: " << angle << " radians ("
                << (angle * 180.0 / PolarTraits<T>::PI) << " degrees)" << std::endl;

            auto result = PolarValue<T>(
                value.magnitude_,
                normalizePhase(value.phase_ + angle)
            );
            debugPrint("Result", result);
            return result;
        }

        // Conversion operations
        static PolarValue<T> toPolar(const RectangularForm<T>& rect) {
            if (isEffectivelyZeroRect(rect)) {
                return PolarValue<T>();
            }

            T magnitude = std::hypot(rect.real, rect.imag);
            T phase = std::atan2(rect.imag, rect.real);

            std::cout << "Rectangular to Polar conversion:"
                << "\n  rect: real=" << rect.real << ", imag=" << rect.imag
                << "\n  result: magnitude=" << magnitude
                << ", phase=" << phase << " ("
                << (phase * 180.0 / PolarTraits<T>::PI) << " degrees)" << std::endl;

            return PolarValue<T>(magnitude, phase);
        }

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

            std::cout << "Polar to Rectangular conversion:"
                << "\n  polar: magnitude=" << polar.magnitude_
                << ", phase=" << polar.phase_
                << " (" << (polar.phase_ * 180.0 / PolarTraits<T>::PI) << " degrees)"
                << "\n  result: real=" << result.real
                << ", imag=" << result.imag << std::endl;

            return result;
        }

    private:
        static T normalizePhase(T phase) noexcept {
            T raw = PolarValue<T>::normalizePhase(phase);
            std::cout << "Phase normalization: " << phase << " -> " << raw
                << " (" << (raw * 180.0 / PolarTraits<T>::PI) << " degrees)" << std::endl;
            return raw;
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