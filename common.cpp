#include "common.h"

#include <algorithm>
#include <cstddef>

void sum_simd_u(const int* a, const int* b, int* c, size_t n) noexcept {
  assert(a && b && c);

  static constexpr auto batch_size = 8uz;

  const auto end_simd = (n / batch_size) * batch_size;
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
  assert(a && b && c);

  static constexpr auto batch_size = 8uz;

  const auto end_simd = (n / batch_size) * batch_size;
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

void sum_simd(const float* a, const float* b, float* c, size_t n) noexcept {
  assert(a && b && c);

  static constexpr auto batch_size = 8uz;

  const auto end_simd = (n / batch_size) * batch_size;
  for (size_t i = 0; i < end_simd; i += batch_size) {
    __m256i x = _mm256_load_ps(a + i);
    __m256i y = _mm256_load_ps(b + i);
    __m256i z = _mm256_add_ps(x, y);

    _mm256_store_ps(c + i, z);
  }

  for (size_t i = end_simd; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

float dot_product_loop(const float* a, const float* b, size_t n) noexcept {
  assert(a && b);

  float res = 0.f;
  for (size_t i = 0; i < n; ++i) {
    res += a[i] * b[i];
  }
  return res;
}

float dot_product_simd(const float* a, const float* b, size_t n) noexcept {
  assert(a && b);

  static constexpr auto batch_size = 8uz;

  const auto end_simd = (n / batch_size) * batch_size;

  __m256 sum = _mm256_setzero_ps();

  for (size_t i = 0; i < end_simd; i += batch_size) {
    __m256 vec_a = _mm256_load_ps(a + i);
    __m256 vec_b = _mm256_load_ps(b + i);
    __m256 mul = _mm256_mul_ps(vec_a, vec_b);
    sum = _mm256_add_ps(sum, mul);
  }

  // horizontal sum the vector to get the final result
  float res[batch_size];
  // store the result back to a temporary array
  _mm256_storeu_ps(res, sum);
  // sum the temporary array
  float final_sum = std::accumulate(res, res + batch_size, 0.f);

  for (size_t i = end_simd; i < n; ++i) {
    final_sum += a[i] * b[i];
  }

  return final_sum;
}

void matmul_loop(const float* a, const float* b, float* c, size_t M, size_t N,
                 size_t K) noexcept {
  assert(a && b && c);
  assert(N > 0 && K > 0 && M > 0);

  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < K; ++j) {
      for (size_t k = 0; k < N; ++k) {
        c[i * K + j] += a[i * N + k] * b[k * K + j];
      }
    }
  }
}

// note: because of the way vectors are stored in memory, we cannot use unaligned loads/stores
// I mean that number of columns may be arbitrary, so we cannot guarantee that load/store is gonna be aligned
void matmul_simd_u(const float* a, const float* b, float* c, size_t M, size_t N,
                 size_t K) noexcept {
  assert(a && b && c);
  assert(N > 0 && K > 0 && M > 0);

  static constexpr auto batch_size = 8uz;

  const auto end_simd = (K / batch_size) * batch_size;

  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < end_simd; j += batch_size) {
      __m256 c_vec = _mm256_setzero_ps();

      for (size_t k = 0; k < N; ++k) {
        // broadcast A[i][k]
        __m256 a_vec = _mm256_broadcast_ss(a + (i * N + k));
        // load 8 elements from row k of B
        __m256 b_vec = _mm256_loadu_ps(b + (k * K + j));
        // multiply A[i][k] with B[k][j...j + 7]
        __m256 product = _mm256_mul_ps(a_vec, b_vec);
        c_vec = _mm256_add_ps(c_vec, product);
      }

      // store the results back into C
      _mm256_storeu_ps(c + (i * K + j), c_vec);
    }
    for (size_t j = end_simd; j < K; ++j) {
      for (size_t k = 0; k < N; ++k) {
        c[i * K + j] += a[i * N + k] * b[k * K + j];
      }
    }
  }
}

[[nodiscard]] size_t count_loop(std::string_view str,
                                std::string_view substr) noexcept {
  assert(!substr.empty());

  if (substr.size() > str.size()) [[unlikely]] {
    return 0;
  }

  size_t count = 0;

  for (size_t i = 0; i <= str.size() - substr.size(); ++i) {
    if (str.substr(i, substr.size()) == substr) {
      ++count;
      i += substr.size() - 1;
    }
  }

  return count;
}

[[nodiscard]] size_t count_simd(std::string_view str,
                                std::string_view substr) noexcept {
  assert(!substr.empty());
  assert(substr.size() <= simd_alignment && "Longer substrings are not supported currently");

  if (substr.size() > str.size() || substr.size() > simd_alignment) [[unlikely]] {
    return 0;
  }

  size_t count = 0;

  const auto batch_size = substr.size();

  const auto end_simd = (str.size() / batch_size) * batch_size;

  __m256i substr_vector = _mm256_load_si256((const __m256i*)(substr.data()));

  // aligned if the substring is 32 bytes long: exact bathes with exact loads
  const bool aligned = substr.size() == simd_alignment;   
  for (size_t i = 0; i < end_simd; i += batch_size) {
    __m256i str_chunk = aligned ? _mm256_load_si256((const __m256i*)(&str[i])) : _mm256_loadu_si256((const __m256i*)(&str[i]));
    __m256i cmp_result = _mm256_cmpeq_epi8(str_chunk, substr_vector);

    int mask = _mm256_movemask_epi8(cmp_result);
    int substr_mask = (1 << substr.size()) - 1;
    count += ((mask & substr_mask) == substr_mask);
  }

  for (size_t i = end_simd; i <= str.size() - substr.size(); ++i) {
    count += (str.substr(i, substr.size()) == substr);
  }

  return count;
}