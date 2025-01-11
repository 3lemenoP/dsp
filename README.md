# DSP Framework (Optimized for Intel i5-6300U)

High-performance digital signal processing framework optimized for Intel i5-6300U CPU architecture.

## System Requirements

- Windows 10 64-bit
- Visual Studio 2022 with C++20 support
- CPU with AVX2 support
- 8GB RAM minimum

## Hardware Optimization

This build is specifically optimized for:
- Intel i5-6300U (2C/4T)
- 2.4GHz base clock
- AVX2 instruction set
- 512KB L2 cache
- 3MB L3 cache
- 8GB DDR3-1867 RAM

## Building

1. Clone the repository:
   \\\bash
   git clone [https://github.com/3lemenoP/dsp]
   cd dsp_framework
   \\\

2. Configure with CMake:
   \\\bash
   cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
   \\\

3. Build:
   \\\bash
   cmake --build build --config Release
   \\\

## Performance Notes

- Threadpool configured for 4 logical processors
- SIMD optimizations using AVX2
- Cache-aware data structures (64-byte cache line)
- Memory alignment optimized for L1 cache (32KB)

## Development Guidelines

1. Memory Management
   - Use aligned allocations (32-byte alignment for AVX2)
   - Keep critical data structures under 32KB (L1 cache size)
   - Optimize for 64-byte cache lines

2. Threading
   - Maximum 4 parallel threads
   - Use lock-free structures where possible
   - Consider CPU core topology in thread assignment

3. SIMD Optimization
   - Leverage AVX2 instructions
   - Align data for vector operations
   - Use SIMD-friendly data layouts

