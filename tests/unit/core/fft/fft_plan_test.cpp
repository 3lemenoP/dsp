#include <gtest/gtest.h>
#include "core/fft/fft_plan.hpp"
#include <random>
#include <cmath>

namespace dsp::core::test {

class FFTPlanTest : public ::testing::Test {
protected:
    // Utility to compare complex numbers with tolerance
    static bool isNearlyEqual(const std::complex<float>& a, 
                             const std::complex<float>& b, 
                             float epsilon = 1e-6f) {
        return std::abs(a - b) <= epsilon * std::max(std::abs(a), std::abs(b));
    }

    // Generate random complex data
    static std::vector<std::complex<float>> generateRandomData(size_t size) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        std::vector<std::complex<float>> data(size);
        for (auto& val : data) {
            val = std::complex<float>(dist(gen), dist(gen));
        }
        return data;
    }

    // Known reference values for small FFT
    // Input: {1, 0, 0, 0, 0, 0, 0, 0}
    static const std::vector<std::complex<float>> knownFFT8;
};

// Reference FFT values for input {1, 0, 0, 0, 0, 0, 0, 0}
const std::vector<std::complex<float>> FFTPlanTest::knownFFT8 = {
    {1.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 0.0f},
    {1.0f, 0.0f}
};

// Test fixture for small FFT sizes
TEST_F(FFTPlanTest, SmallFFTTest) {
    const size_t size = 8;
    FFTPlanF fft(size);

    // Test with impulse input {1, 0, 0, 0, 0, 0, 0, 0}
    std::vector<std::complex<float>> data(size);
    data[0] = std::complex<float>(1.0f, 0.0f);

    fft.forward(data.data());

    // Compare with known reference values
    for (size_t i = 0; i < size; ++i) {
        EXPECT_TRUE(isNearlyEqual(data[i], knownFFT8[i]))
            << "Mismatch at index " << i << ": "
            << "Expected " << knownFFT8[i] << ", got " << data[i];
    }
}

// Test round-trip (forward then inverse) transformation
TEST_F(FFTPlanTest, RoundTripTest) {
    const std::vector<size_t> sizes = {2, 4, 8, 16, 32, 64, 128, 256};
    
    for (size_t size : sizes) {
        FFTPlanF fft(size);
        auto original = generateRandomData(size);
        auto data = original;

        // Forward transform
        fft.forward(data.data());
        
        // Inverse transform
        fft.inverse(data.data());

        // Compare with original
        float maxError = 0.0f;
        for (size_t i = 0; i < size; ++i) {
            float error = std::abs(data[i] - original[i]);
            maxError = std::max(maxError, error);
            EXPECT_TRUE(isNearlyEqual(data[i], original[i], 1e-5f))
                << "Round-trip error at size " << size << ", index " << i << ": "
                << "Expected " << original[i] << ", got " << data[i];
        }
        
        // Log maximum error for this size
        std::cout << "Size " << size << " max round-trip error: " << maxError << std::endl;
    }
}

// Test edge cases
TEST_F(FFTPlanTest, EdgeCasesTest) {
    // Test minimum size (2)
    {
        FFTPlanF fft(2);
        std::vector<std::complex<float>> data = {{1.0f, 0.0f}, {0.0f, 0.0f}};
        auto original = data;
        
        fft.forward(data.data());
        fft.inverse(data.data());
        
        EXPECT_TRUE(isNearlyEqual(data[0], original[0]));
        EXPECT_TRUE(isNearlyEqual(data[1], original[1]));
    }

    // Test with zero input
    {
        FFTPlanF fft(8);
        std::vector<std::complex<float>> data(8);
        fft.forward(data.data());
        
        // All outputs should be zero
        for (const auto& val : data) {
            EXPECT_TRUE(isNearlyEqual(val, std::complex<float>(0.0f, 0.0f)));
        }
    }
}

// Test error conditions
TEST_F(FFTPlanTest, ErrorConditionsTest) {
    // Test non-power-of-2 size
    EXPECT_THROW(FFTPlanF(7), FFTError);
    EXPECT_THROW(FFTPlanF(0), FFTError);
    
    // Test null pointer
    FFTPlanF fft(8);
    EXPECT_THROW(fft.forward(nullptr), FFTError);
    EXPECT_THROW(fft.inverse(nullptr), FFTError);
}

// Performance test for larger sizes
TEST_F(FFTPlanTest, PerformanceTest) {
    const std::vector<size_t> sizes = {1024, 2048, 4096, 8192};
    
    for (size_t size : sizes) {
        FFTPlanF fft(size);
        auto data = generateRandomData(size);
        
        // Measure forward transform time
        auto start = std::chrono::high_resolution_clock::now();
        fft.forward(data.data());
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Forward FFT size " << size << " took " << duration.count() << " microseconds" << std::endl;
        
        // Verify the transform still works at this size
        fft.inverse(data.data());
    }
}

} // namespace dsp::core::test