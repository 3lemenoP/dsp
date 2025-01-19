#ifndef DSP_CORE_FFT_PLAN_HPP
#define DSP_CORE_FFT_PLAN_HPP

#include <complex>
#include <memory>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace dsp::core {

/**
 * @brief Configuration options for FFT plan creation
 */
struct FFTPlanConfig {
    bool inPlace{true};      // Whether transforms are performed in-place
    bool useAVX{true};       // Use AVX instructions when available
    bool doublePrec{false};  // Use double precision
};

/**
 * @brief Exception class for FFT-related errors
 */
class FFTError : public std::runtime_error {
public:
    explicit FFTError(const char* msg) : std::runtime_error(msg) {}
};

/**
 * @brief Class for FFT plan management and execution
 * 
 * PERFORMANCE: Pre-computes and caches twiddle factors and bit reversal indices
 * THREAD-SAFETY: Multiple threads can safely execute transforms using the same plan
 * MEMORY: Aligned allocations for SIMD optimization
 */
template<typename T>
class FFTPlan {
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>,
                 "FFTPlan only supports float or double");

public:
    /**
     * @brief Construct a new FFTPlan
     * 
     * @param size FFT size (must be power of 2)
     * @param config Configuration options
     * @throws FFTError if size is not power of 2 or allocation fails
     */
    explicit FFTPlan(size_t size, const FFTPlanConfig& config = FFTPlanConfig{});

    /**
     * @brief Get the size of the FFT
     */
    size_t size() const noexcept { return size_; }

    /**
     * @brief Check if the plan is for in-place transforms
     */
    bool isInPlace() const noexcept { return config_.inPlace; }

    /**
     * @brief Execute forward FFT
     * 
     * @param data Input/output data buffer
     * @throws FFTError if data is null or execution fails
     * 
     * PERFORMANCE: Optimized for power-of-2 sizes using butterfly operations
     * MEMORY: In-place operation, no additional allocations during transform
     */
    void forward(std::complex<T>* data);

    /**
     * @brief Execute inverse FFT
     * 
     * @param data Input/output data buffer
     * @throws FFTError if data is null or execution fails
     * 
     * PERFORMANCE: Optimized for power-of-2 sizes using butterfly operations
     * MEMORY: In-place operation, no additional allocations during transform
     */
    void inverse(std::complex<T>* data);

private:
    // PERFORMANCE: Cache line aligned storage for twiddle factors
    alignas(64) std::vector<std::complex<T>> twiddles_;
    // PERFORMANCE: Cache line aligned storage for bit reversal indices
    alignas(64) std::vector<size_t> bitReversalIndices_;
    
    size_t size_;              // Size of the FFT
    size_t stages_;            // Number of butterfly stages (log2(size))
    FFTPlanConfig config_;     // Configuration options
    T scaleFactor_;           // Scaling factor for inverse transform

    // Helper functions for initialization
    void initTwiddles();
    void initBitReversalIndices();
    
    // Core FFT computation functions
    void bitReverse(std::complex<T>* data) const noexcept;
    void computeButterflies(std::complex<T>* data, bool inverse) const noexcept;
    void butterflyPass(std::complex<T>* data, size_t stage, bool inverse) const noexcept;
    
    // Utility functions
    static bool isPowerOfTwo(size_t value) noexcept;
    static size_t log2(size_t value) noexcept;
    static size_t reverseBits(size_t value, size_t bits) noexcept;
};

// Common type aliases
using FFTPlanF = FFTPlan<float>;
using FFTPlanD = FFTPlan<double>;

} // namespace dsp::core

#endif // DSP_CORE_FFT_PLAN_HPP