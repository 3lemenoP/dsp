#include <gtest/gtest.h>
#include "dsp/polar/types.hpp"
#include <cmath>
#include <limits>
#include <random>

namespace dsp::polar::test {

    class PolarValueTest : public ::testing::Test {
    protected:
        static constexpr double PI = PolarTraits<double>::PI;
        static constexpr double EPSILON = PolarTraits<double>::EPSILON;
        static constexpr double MIN_MAG = PolarTraits<double>::MIN_MAGNITUDE;
        static constexpr double MAX_MAG = PolarTraits<double>::MAX_MAGNITUDE;

        template<typename T>
        static bool nearlyEqual(T a, T b, T epsilon = PolarTraits<T>::EPSILON) {
            if (std::abs(a) <= epsilon && std::abs(b) <= epsilon) {
                return true;  // Both effectively zero
            }
            return std::abs(a - b) <= epsilon * std::max(std::abs(a), std::abs(b));
        }

        template<typename T>
        static bool nearlyPhaseEqual(T phaseA, T phaseB, T epsilon = 1e-6) {
            T diff = std::fmod(phaseA - phaseB, T(2.0) * T(PI));
            if (diff > T(PI)) {
                diff -= T(2.0) * T(PI);
            }
            else if (diff < -T(PI)) {
                diff += T(2.0) * T(PI);
            }
            return std::abs(diff) <= epsilon;
        }

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
        EXPECT_THROW(PolarDouble(-1.0, 0.0), PolarError);
        EXPECT_THROW(PolarDouble(MAX_MAG * 2.0, 0.0), PolarError);
        EXPECT_NO_THROW(PolarDouble(MIN_MAG, 0.0));
        EXPECT_NO_THROW(PolarDouble(MAX_MAG * 0.9, 0.0));
    }

    // Phase Normalization Tests
    TEST_F(PolarValueTest, PhaseNormalization) {
        {
            PolarDouble value(1.0, 3 * PI);
            EXPECT_TRUE(nearlyPhaseEqual(value.getPhase(), -PI));
        }

        {
            PolarDouble value(1.0, -3 * PI);
            EXPECT_TRUE(nearlyPhaseEqual(value.getPhase(), PI));
        }

        {
            PolarDouble value1(1.0, 2 * PI);
            EXPECT_TRUE(nearlyPhaseEqual(value1.getPhase(), 0.0));

            PolarDouble value2(1.0, -2 * PI);
            EXPECT_TRUE(nearlyPhaseEqual(value2.getPhase(), 0.0));
        }

        {
            PolarDouble value1(1.0, PI - EPSILON / 2);
            EXPECT_TRUE(nearlyEqual(value1.getPhase(), PI - EPSILON / 2));

            PolarDouble value2(1.0, -PI + EPSILON / 2);
            EXPECT_TRUE(nearlyEqual(value2.getPhase(), -PI + EPSILON / 2));
        }
    }

    // Comparison Tests
    TEST_F(PolarValueTest, Equality) {
        {
            PolarDouble a(1.0, PI / 4);
            PolarDouble b(1.0, PI / 4);
            EXPECT_EQ(a, b);
        }

        {
            PolarDouble a(1.0, PI / 4);
            PolarDouble b(1.0, -7 * PI / 4);
            EXPECT_EQ(a, b);
        }

        {
            PolarDouble a(MIN_MAG / 2.0, PI / 4);
            PolarDouble b(MIN_MAG / 3.0, PI / 3);
            EXPECT_EQ(a, b);  // Both effectively zero
        }
    }

    // Edge Case Tests
    TEST_F(PolarValueTest, EdgeCases) {
        {
            PolarDouble tiny(MIN_MAG / 2.0, PI / 4);
            EXPECT_TRUE(tiny.isZero());
        }

        {
            PolarDouble value(1.0, PI);
            EXPECT_TRUE(nearlyEqual(value.getPhase(), PI));
        }
    }

    // Accessors and Mutators Tests
    TEST_F(PolarValueTest, AccessorsAndMutators) {
        PolarDouble value(1.0, PI / 4);

        // Test getters
        EXPECT_EQ(value.getMagnitude(), 1.0);
        EXPECT_TRUE(nearlyEqual(value.getPhase(), PI / 4));

        // Test setMagnitude
        value.setMagnitude(2.0);
        EXPECT_EQ(value.getMagnitude(), 2.0);
        EXPECT_THROW(value.setMagnitude(-1.0), PolarError);
        EXPECT_THROW(value.setMagnitude(MAX_MAG * 2.0), PolarError);

        // Test setPhase
        value.setPhase(PI / 2);
        EXPECT_TRUE(nearlyEqual(value.getPhase(), PI / 2));
    }

    // Float Template Tests
    TEST_F(PolarValueTest, FloatTemplateInstantiation) {
        PolarFloat value(1.0f, PolarTraits<float>::PI / 2);
        EXPECT_TRUE(nearlyEqual(value.getMagnitude(), 1.0f));
        EXPECT_TRUE(nearlyEqual(value.getPhase(), PolarTraits<float>::PI / 2));
    }

} // namespace dsp::polar::test