#include <x86intrin.h>
#include <bits/stdc++.h>

#include <cstdlib>
#include <stdexcept>
#include <cassert>
#include <format>
#include <string_view>

// unaligned
void sum_simd_u(const int* a, const int* b, int* c, size_t n) noexcept;

// aligned
void sum_simd(const int* a, const int* b, int* c, size_t n) noexcept;

// aligned
void sum_simd(const float* a, const float* b, float* c, size_t n) noexcept;

template <class T>
void sum_loop(const T* a, const T* b, T* c, size_t n) noexcept {
  assert(a && b && c);

  for (size_t i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

[[nodiscard]] float dot_product_loop(const float* a, const float* b, size_t n) noexcept;

// aligned
[[nodiscard]] float dot_product_simd(const float* a, const float* b, size_t n) noexcept;

// M: Number of rows in matrix A (and the number of rows in the result matrix C)
// N: Number of columns in matrix A (and the number of rows in matrix B)
// K: Number of columns in matrix B (and the number of columns in the result matrix C)
void matmul_loop(const float* a, const float* b, float* c, size_t M, size_t N, size_t K) noexcept;

void matmul_simd_u(const float* a, const float* b, float* c, size_t M, size_t N, size_t K) noexcept;

/// @return number of occurences of substring in string
[[nodiscard]] size_t count_loop(std::string_view str, std::string_view substr) noexcept;

// aligned if substr.size() is 32 bytes exactly
// does not support substr with size > 32
[[nodiscard]] size_t count_simd(const char* str, const char* substr, size_t n) noexcept;
[[nodiscard]] size_t count_simd(std::string_view str, std::string_view substr) noexcept;

template <class T>
T* allocate_aligned_array(size_t size, size_t alignment) {
  assert(size > 0);

  T* res = nullptr;

#ifdef _WIN32
#error "Windows is not implemented"
#else
  if (auto code = posix_memalign((void**)&res, alignment, size * sizeof(T)); code != 0) {
    throw std::runtime_error(std::format("Memory allocation failed, code: {}", code));
  }
#endif

  return res;
}

template <class T>
void free_aligned_array(T* ptr) {
#ifdef _WIN32
#error "Windows is not implemented"
#else
  std::free(ptr);
#endif
}