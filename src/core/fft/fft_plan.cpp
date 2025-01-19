#include "core/fft/fft_plan.hpp"
#include <cassert>
#include <cmath>
#include <numbers> // For std::numbers::pi in C++20

namespace dsp::core {

template<typename T>
FFTPlan<T>::FFTPlan(size_t size, const FFTPlanConfig& config)
    : size_(size)
    , config_(config)
    , scaleFactor_(T(1) / static_cast<T>(size)) {
    
    if (!isPowerOfTwo(size)) {
        throw FFTError("FFT size must be a power of 2");
    }

    stages_ = log2(size);
    
    // Initialize twiddle factors and bit reversal indices
    initTwiddles();
    initBitReversalIndices();
}

template<typename T>
void FFTPlan<T>::forward(std::complex<T>* data) {
    if (!data) {
        throw FFTError("Null data pointer");
    }

    bitReverse(data);
    computeButterflies(data, false);
}

template<typename T>
void FFTPlan<T>::inverse(std::complex<T>* data) {
    if (!data) {
        throw FFTError("Null data pointer");
    }

    bitReverse(data);
    computeButterflies(data, true);

    // Scale the result
    for (size_t i = 0; i < size_; ++i) {
        data[i] *= scaleFactor_;
    }
}

template<typename T>
void FFTPlan<T>::initTwiddles() {
    // Allocate twiddle factors for N/2 points
    twiddles_.resize(size_ / 2);
    
    constexpr T pi = std::numbers::pi_v<T>;
    for (size_t k = 0; k < size_ / 2; ++k) {
        T angle = -2 * pi * k / static_cast<T>(size_);
        twiddles_[k] = std::complex<T>(std::cos(angle), std::sin(angle));
    }
}

template<typename T>
void FFTPlan<T>::initBitReversalIndices() {
    bitReversalIndices_.resize(size_);
    
    for (size_t i = 0; i < size_; ++i) {
        bitReversalIndices_[i] = reverseBits(i, stages_);
    }
}

template<typename T>
void FFTPlan<T>::bitReverse(std::complex<T>* data) const noexcept {
    // Only swap elements where i < j to avoid double-swapping
    for (size_t i = 0; i < size_; ++i) {
        size_t j = bitReversalIndices_[i];
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

template<typename T>
void FFTPlan<T>::computeButterflies(std::complex<T>* data, bool inverse) const noexcept {
    // Perform butterfly operations for each stage
    for (size_t stage = 0; stage < stages_; ++stage) {
        butterflyPass(data, stage, inverse);
    }
}

template<typename T>
void FFTPlan<T>::butterflyPass(std::complex<T>* data, size_t stage, bool inverse) const noexcept {
    const size_t butterflySize = size_t(1) << stage;
    const size_t numGroups = size_ >> (stage + 1);
    const size_t groupStep = butterflySize << 1;
    
    for (size_t group = 0; group < numGroups; ++group) {
        size_t groupOffset = group * groupStep;
        size_t twiddle_idx = 0;
        const size_t twiddle_step = size_ >> (stage + 1);

        for (size_t butterfly = 0; butterfly < butterflySize; ++butterfly) {
            size_t i = groupOffset + butterfly;
            size_t j = i + butterflySize;
            
            std::complex<T> twiddle = twiddles_[twiddle_idx];
            if (inverse) {
                twiddle = std::conj(twiddle);
            }

            std::complex<T> temp = data[j] * twiddle;
            data[j] = data[i] - temp;
            data[i] = data[i] + temp;
            
            twiddle_idx += twiddle_step;
        }
    }
}

template<typename T>
bool FFTPlan<T>::isPowerOfTwo(size_t value) noexcept {
    return value > 0 && (value & (value - 1)) == 0;
}

template<typename T>
size_t FFTPlan<T>::log2(size_t value) noexcept {
    return static_cast<size_t>(std::log2(static_cast<double>(value)));
}

template<typename T>
size_t FFTPlan<T>::reverseBits(size_t value, size_t bits) noexcept {
    size_t result = 0;
    for (size_t i = 0; i < bits; ++i) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

// Explicit template instantiations
template class FFTPlan<float>;
template class FFTPlan<double>;

} // namespace dsp::core