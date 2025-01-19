#include <gtest/gtest.h>
#include <dsp/polar/operations.hpp>
#include <cmath>
#include <random>

namespace dsp::polar::test {

    class PolarOperationsTest : public ::testing::Test {
    protected:
        static constexpr double PI = PolarTraits<double>::PI;
        static constexpr double EPSILON = PolarTraits<double>::EPSILON;
        static constexpr double MIN_MAG = PolarTraits<double>::MIN_MAGNITUDE;
        static constexpr double MAX_MAG = PolarTraits<double>::MAX_MAGNITUDE;

        bool areRectangularFormsClose(const RectangularForm<double>& a,
            const RectangularForm<double>& b,
            double tolerance = EPSILON) {
            return std::abs(a.real - b.real) <= tolerance &&
                std::abs(a.imag - b.imag) <= tolerance;
        }

        bool arePolarValuesClose(const PolarValue<double>& a,
            const PolarValue<double>& b,
            double tolerance = EPSILON) {
            if (a.isZero() || b.isZero()) {
                return a.isZero() && b.isZero();
            }

            bool magClose = std::abs(a.getMagnitude() - b.getMagnitude()) <=
                tolerance * std::max(a.getMagnitude(), b.getMagnitude());

            double phaseDiff = std::fmod(a.getPhase() - b.getPhase(), 2 * PI);
            if (phaseDiff > PI) phaseDiff -= 2 * PI;
            else if (phaseDiff <= -PI) phaseDiff += 2 * PI;

            return magClose && std::abs(phaseDiff) <= tolerance;
        }

        static PolarDouble randomPolar(double maxMag = 1000.0) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<double> mag_dist(0.0, maxMag);
            static std::uniform_real_distribution<double> phase_dist(-PI, PI);
            return PolarDouble(mag_dist(gen), phase_dist(gen));
        }
    };

    // Conversion Tests
    TEST_F(PolarOperationsTest, RectangularToPolarConversion) {
        // Basic conversions
        EXPECT_TRUE(arePolarValuesClose(
            toPolar(RectangularForm<double>{1.0, 0.0}),
            PolarValue<double>(1.0, 0.0)
        ));

        EXPECT_TRUE(arePolarValuesClose(
            toPolar(RectangularForm<double>{0.0, 1.0}),
            PolarValue<double>(1.0, PI / 2)
        ));

        // Zero case
        EXPECT_TRUE(arePolarValuesClose(
            toPolar(RectangularForm<double>{0.0, 0.0}),
            PolarValue<double>(0.0, 0.0)
        ));

        // 45-degree case
        auto val45 = toPolar(RectangularForm<double>{1.0, 1.0});
        EXPECT_NEAR(val45.getMagnitude(), std::sqrt(2.0), EPSILON);
        EXPECT_NEAR(val45.getPhase(), PI / 4, EPSILON);
    }

    TEST_F(PolarOperationsTest, PolarToRectangularConversion) {
        // Basic conversions
        EXPECT_TRUE(areRectangularFormsClose(
            toRectangular(PolarValue<double>(1.0, 0.0)),
            RectangularForm<double>{1.0, 0.0}
        ));

        EXPECT_TRUE(areRectangularFormsClose(
            toRectangular(PolarValue<double>(1.0, PI / 2)),
            RectangularForm<double>{0.0, 1.0}
        ));

        // Zero magnitude
        EXPECT_TRUE(areRectangularFormsClose(
            toRectangular(PolarValue<double>(0.0, PI / 4)),
            RectangularForm<double>{0.0, 0.0}
        ));
    }

    TEST_F(PolarOperationsTest, ConversionRoundTrip) {
        std::vector<double> angles = { 0.0, PI / 6, PI / 4, PI / 3, PI / 2, PI };
        std::vector<double> magnitudes = { 0.0, 0.5, 1.0, 2.0 };

        for (double mag : magnitudes) {
            for (double angle : angles) {
                PolarValue<double> original(mag, angle);
                auto rectangular = toRectangular(original);
                auto roundTrip = toPolar(rectangular);
                EXPECT_TRUE(arePolarValuesClose(original, roundTrip))
                    << "Failed at magnitude=" << mag << ", angle=" << angle;
            }
        }
    }

    // Core Operation Tests
    TEST_F(PolarOperationsTest, MultiplicationBasic) {
        PolarDouble a(2.0, PI / 4);
        PolarDouble b(3.0, PI / 3);
        auto result = PolarOperations<double>::multiply(a, b);

        EXPECT_NEAR(result.getMagnitude(), 6.0, EPSILON);
        EXPECT_NEAR(result.getPhase(), 7 * PI / 12, EPSILON);
    }

    TEST_F(PolarOperationsTest, MultiplicationWithZero) {
        PolarDouble a(2.0, PI / 4);
        PolarDouble zero;
        auto result = PolarOperations<double>::multiply(a, zero);
        EXPECT_TRUE(result.isZero());
    }

    TEST_F(PolarOperationsTest, DivisionBasic) {
        PolarDouble a(6.0, PI / 2);
        PolarDouble b(2.0, PI / 6);
        auto result = PolarOperations<double>::divide(a, b);

        EXPECT_NEAR(result.getMagnitude(), 3.0, EPSILON);
        EXPECT_NEAR(result.getPhase(), PI / 3, EPSILON);
    }

    TEST_F(PolarOperationsTest, DivisionByZero) {
        PolarDouble a(1.0, 0.0);
        PolarDouble zero;
        EXPECT_THROW(PolarOperations<double>::divide(a, zero), PolarError);
    }

    // Addition and Subtraction Tests
    TEST_F(PolarOperationsTest, Addition) {
        // Test adding opposite-phase vectors
        auto sum = PolarOperations<double>::add(
            PolarValue<double>(1.0, 0.0),
            PolarValue<double>(1.0, PI)
        );
        
        // Use relative tolerance for magnitude comparison
        double tolerance = std::max(sum.getMagnitude(), 1.0) * PolarTraits<double>::EPSILON * 100.0;
        EXPECT_NEAR(sum.getMagnitude(), 0.0, tolerance) 
            << "Sum magnitude should be effectively zero";
    }

    TEST_F(PolarOperationsTest, Subtraction) {
        // Orthogonal values
        auto diff = PolarOperations<double>::subtract(
            PolarValue<double>(1.0, 0.0),
            PolarValue<double>(1.0, PI / 2)
        );

        // Magnitude check remains the same
        EXPECT_NEAR(diff.getMagnitude(), std::sqrt(2.0), EPSILON);

        // Phase check should account for the correct quadrant (-π/4 is correct)
        EXPECT_NEAR(diff.getPhase(), -PI / 4, EPSILON)
            << "Phase should be -45 degrees (vector pointing southeast)";
    }

    // Special Operations Tests
    TEST_F(PolarOperationsTest, MultiplyConjugate) {
        PolarDouble a(2.0, PI / 4);
        PolarDouble b(3.0, PI / 3);
        auto result = PolarOperations<double>::multiplyConjugate(a, b);

        EXPECT_NEAR(result.getMagnitude(), 6.0, EPSILON);
        EXPECT_NEAR(result.getPhase(), PI / 4 - PI / 3, EPSILON);
    }

    TEST_F(PolarOperationsTest, Rotation) {
        PolarDouble original(1.0, 0.0);

        // 90-degree rotation
        auto rotated = PolarOperations<double>::rotate(original, PI / 2);
        EXPECT_NEAR(rotated.getMagnitude(), 1.0, EPSILON);
        EXPECT_NEAR(rotated.getPhase(), PI / 2, EPSILON);

        // Full rotation
        rotated = PolarOperations<double>::rotate(original, 2 * PI);
        EXPECT_TRUE(arePolarValuesClose(rotated, original));
    }

    TEST_F(PolarOperationsTest, Scaling) {
        PolarDouble value(2.0, PI / 4);
        auto scaled = PolarOperations<double>::scale(value, -2.0);

        // Magnitude check
        EXPECT_NEAR(scaled.getMagnitude(), 4.0, EPSILON);

        // Phase check should handle wraparound
        // 5π/4 and -3π/4 are equivalent angles
        double expected_phase = -3 * PI / 4; // Canonical form in [-π, π]
        
        // Compare phases accounting for 2π periodicity
        double phase_diff = std::fmod(std::abs(scaled.getPhase() - expected_phase), 2 * PI);
        phase_diff = std::min(phase_diff, 2 * PI - phase_diff);
        EXPECT_NEAR(phase_diff, 0.0, EPSILON)
            << "Phases should be equivalent modulo 2π";
    }

    // Property-Based Tests
    TEST_F(PolarOperationsTest, MultiplicationProperties) {
        for (int i = 0; i < 100; ++i) {
            auto a = randomPolar();
            auto b = randomPolar();
            auto c = randomPolar();

            // Associativity: (a * b) * c == a * (b * c)
            auto result1 = PolarOperations<double>::multiply(
                PolarOperations<double>::multiply(a, b), c);
            auto result2 = PolarOperations<double>::multiply(
                a, PolarOperations<double>::multiply(b, c));
            EXPECT_TRUE(arePolarValuesClose(result1, result2));

            // Identity: a * 1 == a
            auto identity = PolarDouble(1.0, 0.0);
            auto result3 = PolarOperations<double>::multiply(a, identity);
            EXPECT_TRUE(arePolarValuesClose(result3, a));

            // Zero property: a * 0 == 0
            auto zero = PolarDouble();
            auto result4 = PolarOperations<double>::multiply(a, zero);
            EXPECT_TRUE(result4.isZero());
        }
    }

    // Operator Tests
    TEST_F(PolarOperationsTest, OperatorOverloads) {
        PolarDouble a(2.0, PI / 4);
        PolarDouble b(3.0, PI / 3);

        // Test operator+
        auto sum = a + b;
        auto expectedSum = PolarOperations<double>::add(a, b);
        EXPECT_TRUE(arePolarValuesClose(sum, expectedSum));

        // Test operator-
        auto diff = a - b;
        auto expectedDiff = PolarOperations<double>::subtract(a, b);
        EXPECT_TRUE(arePolarValuesClose(diff, expectedDiff));

        // Test operator*
        auto prod = a * b;
        auto expectedProd = PolarOperations<double>::multiply(a, b);
        EXPECT_TRUE(arePolarValuesClose(prod, expectedProd));

        // Test operator/
        auto quot = a / b;
        auto expectedQuot = PolarOperations<double>::divide(a, b);
        EXPECT_TRUE(arePolarValuesClose(quot, expectedQuot));

        // Test scalar operators
        auto scaledRight = a * 2.0;
        auto scaledLeft = 2.0 * a;
        auto expectedScaled = PolarOperations<double>::scale(a, 2.0);
        EXPECT_TRUE(arePolarValuesClose(scaledRight, expectedScaled));
        EXPECT_TRUE(arePolarValuesClose(scaledLeft, expectedScaled));
    }

} // namespace dsp::polar::test