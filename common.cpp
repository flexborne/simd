#include "common.h"

void sum_loop(const int* a, const int* b, int* c, size_t n) noexcept {
  assert(a && b && c);

  for (size_t i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

void sum_simd_u(const int* a, const int* b, int* c, size_t n) noexcept {
  assert(a && b && c);

  static constexpr auto batch_size = 8uz;

  const auto end_simd =  (n / batch_size) * batch_size;
  for (size_t i = 0; i < end_simd; i += batch_size) {
    __m256i x = _mm256_loadu_si256((__m256i*)(a + i));
    __m256i y = _mm256_loadu_si256((__m256i*)(b + i));
    __m256i z = _mm256_add_epi32(x, y);

    _mm256_storeu_si256((__m256i*)(c + i), z);
  }

  for (size_t i = end_simd; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

void sum_simd(const int* a, const int* b, int* c, size_t n) noexcept {
  static constexpr auto batch_size = 8uz;

  const auto end_simd =  (n / batch_size) * batch_size;
  for (size_t i = 0; i < end_simd; i += batch_size) {
    __m256i x = _mm256_load_si256((__m256i*)(a + i));
    __m256i y = _mm256_load_si256((__m256i*)(b + i));
    __m256i z = _mm256_add_epi32(x, y);

    _mm256_store_si256((__m256i*)(c + i), z);
  }

  for (size_t i = end_simd; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}