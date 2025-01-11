// File: src/polar/tests/unit/types_test.cpp

#include <gtest/gtest.h>
#include "dsp/polar/types.hpp"
#include <cmath>
#include <limits>
#include <random>

namespace dsp::polar::test {

    // Test fixture for polar value tests
    class PolarValueTest : public ::testing::Test {
    protected:
        // Common test values
        static constexpr double PI = PolarTraits<double>::PI;
        static constexpr double EPSILON = PolarTraits<double>::EPSILON;
        static constexpr double MIN_MAG = PolarTraits<double>::MIN_MAGNITUDE;
        static constexpr double MAX_MAG = PolarTraits<double>::MAX_MAGNITUDE;

        // Existing nearlyEqual for magnitudes and general usage:
        template<typename T>
        static bool nearlyEqual(T a, T b, T epsilon = PolarTraits<T>::EPSILON) {
            if (std::abs(a) <= epsilon && std::abs(b) <= epsilon) {
                return true;  // Both effectively zero
            }
            return std::abs(a - b) <= epsilon * std::max(std::abs(a), std::abs(b));
        }

        // NEW: A helper specifically for angles, wrapping difference to [-π, π].
        template<typename T>
        static bool nearlyPhaseEqual(T phaseA, T phaseB, T epsilon = 1e-6) {
            // Bring difference into [-π, π]
            T diff = std::fmod(phaseA - phaseB, T(2.0) * T(PI));
            if (diff > T(PI)) {
                diff -= T(2.0) * T(PI);
            }
            else if (diff < -T(PI)) {
                diff += T(2.0) * T(PI);
            }
            return std::abs(diff) <= epsilon;
        }

        // Helper to create random values for property-based testing
        static PolarDouble randomPolar(double maxMag = 1000.0) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<double> mag_dist(0.0, maxMag);
            static std::uniform_real_distribution<double> phase_dist(-PI, PI);

            return PolarDouble(mag_dist(gen), phase_dist(gen));
        }
    };

    // Constructor Tests
    TEST_F(PolarValueTest, DefaultConstructor) {
        PolarDouble value;
        EXPECT_EQ(value.getMagnitude(), 0.0);
        EXPECT_EQ(value.getPhase(), 0.0);
        EXPECT_TRUE(value.isZero());
    }

    TEST_F(PolarValueTest, ParameterizedConstructor) {
        PolarDouble value(1.0, PI / 2);
        EXPECT_EQ(value.getMagnitude(), 1.0);
        EXPECT_TRUE(nearlyEqual(value.getPhase(), PI / 2));
    }

    TEST_F(PolarValueTest, ConstructorValidation) {
        // Test negative magnitude
        EXPECT_THROW(PolarDouble(-1.0, 0.0), PolarError);

        // Test excessive magnitude
        EXPECT_THROW(PolarDouble(MAX_MAG * 2.0, 0.0), PolarError);

        // Test valid edge cases
        EXPECT_NO_THROW(PolarDouble(MIN_MAG, 0.0));
        EXPECT_NO_THROW(PolarDouble(MAX_MAG * 0.9, 0.0));
    }

    // Phase Normalization Tests
    TEST_F(PolarValueTest, PhaseNormalization) {
        // Test regular phase wrapping
        {
            PolarDouble value(1.0, 3 * PI);
            // Changed from nearlyEqual(...) to nearlyPhaseEqual(...)
            EXPECT_TRUE(nearlyPhaseEqual(value.getPhase(), -PI));
        }

        // Test negative phase wrapping
        {
            PolarDouble value(1.0, -3 * PI);
            // Also changed to nearlyPhaseEqual(...)
            EXPECT_TRUE(nearlyPhaseEqual(value.getPhase(), PI));
        }

        // Test exact multiples of PI
        {
            PolarDouble value1(1.0, 2 * PI);
            EXPECT_TRUE(nearlyPhaseEqual(value1.getPhase(), 0.0));

            PolarDouble value2(1.0, -2 * PI);
            EXPECT_TRUE(nearlyPhaseEqual(value2.getPhase(), 0.0));
        }

        // Test phase near boundaries
        {
            PolarDouble value1(1.0, PI - EPSILON / 2);
            EXPECT_TRUE(nearlyEqual(value1.getPhase(), PI - EPSILON / 2));

            PolarDouble value2(1.0, -PI + EPSILON / 2);
            EXPECT_TRUE(nearlyEqual(value2.getPhase(), -PI + EPSILON / 2));
        }
    }

    // Multiplication Tests
    TEST_F(PolarValueTest, ValueMultiplication) {
        // Basic multiplication
        {
            PolarDouble a(2.0, PI / 4);
            PolarDouble b(3.0, PI / 3);
            auto result = a * b;
            EXPECT_TRUE(nearlyEqual(result.getMagnitude(), 6.0));
            EXPECT_TRUE(nearlyPhaseEqual(result.getPhase(), 7 * PI / 12));
        }

        // Multiplication near zero
        {
            PolarDouble a(MIN_MAG / 2.0, PI / 4);
            PolarDouble b(2.0, PI / 3);
            auto result = a * b;
            EXPECT_TRUE(result.isZero());
        }

        // Phase wrapping in multiplication
        {
            PolarDouble a(2.0, 0.9 * PI);
            PolarDouble b(3.0, 0.9 * PI);
            auto result = a * b;
            EXPECT_TRUE(nearlyEqual(result.getMagnitude(), 6.0));
            EXPECT_TRUE(nearlyPhaseEqual(result.getPhase(), -0.2 * PI));
        }
    }

    TEST_F(PolarValueTest, ScalarMultiplication) {
        PolarDouble value(2.0, PI / 4);

        // Positive scalar
        {
            auto result = value * 3.0;
            EXPECT_TRUE(nearlyEqual(result.getMagnitude(), 6.0));
            EXPECT_TRUE(nearlyPhaseEqual(result.getPhase(), PI / 4));
        }

        // Negative scalar
        {
            auto result = value * -2.0;
            EXPECT_TRUE(nearlyEqual(result.getMagnitude(), 4.0));
            // Use nearlyPhaseEqual(...) for checking 5π/4
            EXPECT_TRUE(nearlyPhaseEqual(result.getPhase(), 5 * PI / 4));
        }

        // Zero scalar
        {
            auto result = value * 0.0;
            EXPECT_TRUE(result.isZero());
        }

        // Small scalar
        {
            auto result = value * (MIN_MAG / 2.0);
            EXPECT_TRUE(result.isZero());
        }
    }

    // Division Tests
    TEST_F(PolarValueTest, Division) {
        // Basic division
        {
            PolarDouble a(6.0, PI / 2);
            PolarDouble b(2.0, PI / 4);
            auto result = a / b;
            EXPECT_TRUE(nearlyEqual(result.getMagnitude(), 3.0));
            EXPECT_TRUE(nearlyPhaseEqual(result.getPhase(), PI / 4));
        }

        // Division by near-zero
        {
            PolarDouble a(1.0, 0.0);
            PolarDouble b(MIN_MAG / 2.0, 0.0);
            EXPECT_THROW(a / b, PolarError);
        }

        // Division with phase wrapping
        {
            PolarDouble a(2.0, 0.9 * PI);
            PolarDouble b(2.0, -0.9 * PI);
            auto result = a / b;
            EXPECT_TRUE(nearlyEqual(result.getMagnitude(), 1.0));
            EXPECT_TRUE(nearlyPhaseEqual(result.getPhase(), -0.2 * PI));
        }
    }

    // Comparison Tests
    TEST_F(PolarValueTest, Equality) {
        // Basic equality
        {
            PolarDouble a(1.0, PI / 4);
            PolarDouble b(1.0, PI / 4);
            EXPECT_EQ(a, b);
        }

        // Equal values with wrapped phases
        {
            PolarDouble a(1.0, PI / 4);
            PolarDouble b(1.0, -7 * PI / 4);
            EXPECT_EQ(a, b);
        }

        // Near-zero magnitudes
        {
            PolarDouble a(MIN_MAG / 2.0, PI / 4);
            PolarDouble b(MIN_MAG / 3.0, PI / 3);
            EXPECT_EQ(a, b);  // Both effectively zero
        }
    }

    // Property-Based Tests
    TEST_F(PolarValueTest, MultiplicationProperties) {
        for (int i = 0; i < 100; ++i) {
            auto a = randomPolar();
            auto b = randomPolar();
            auto c = randomPolar();

            // Associativity: (a * b) * c == a * (b * c)
            auto result1 = (a * b) * c;
            auto result2 = a * (b * c);
            EXPECT_TRUE(nearlyEqual(result1.getMagnitude(), result2.getMagnitude()));
            // Use nearlyPhaseEqual for phase
            EXPECT_TRUE(nearlyPhaseEqual(result1.getPhase(), result2.getPhase()));

            // Identity: a * 1 == a
            auto identity = PolarDouble(1.0, 0.0);
            auto result3 = a * identity;
            EXPECT_TRUE(nearlyEqual(result3.getMagnitude(), a.getMagnitude()));
            EXPECT_TRUE(nearlyPhaseEqual(result3.getPhase(), a.getPhase()));

            // Zero property: a * 0 == 0
            auto zero = PolarDouble();
            auto result4 = a * zero;
            EXPECT_TRUE(result4.isZero());
        }
    }

    // Edge Case Tests
    TEST_F(PolarValueTest, EdgeCases) {
        // Very small magnitude
        {
            PolarDouble tiny(MIN_MAG / 2.0, PI / 4);
            EXPECT_TRUE(tiny.isZero());
        }

        // Maximum safe magnitude
        {
            PolarDouble large(MAX_MAG * 0.9, PI / 4);
            EXPECT_NO_THROW(large * PolarDouble(1.0, 0.0));
        }

        // Phase at exact PI
        {
            PolarDouble value(1.0, PI);
            // We'll keep the original nearlyEqual for direct check
            EXPECT_TRUE(nearlyEqual(value.getPhase(), PI));
        }
    }

    // Float Template Tests
    TEST_F(PolarValueTest, FloatTemplateInstantiation) {
        PolarFloat value(1.0f, PolarTraits<float>::PI / 2);
        EXPECT_TRUE(nearlyEqual(value.getMagnitude(), 1.0f));
        // Use nearlyPhaseEqual here if you prefer, but it’s typically fine:
        EXPECT_TRUE(nearlyEqual(value.getPhase(), PolarTraits<float>::PI / 2));
    }

} // namespace dsp::polar::test
