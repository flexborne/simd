#include <x86intrin.h>
#include <bits/stdc++.h>

#include <cstdlib>
#include <stdexcept>

// unaligned
void sum_simd_u(const int* a, const int* b, int* c, size_t n) noexcept;

// aligned
void sum_simd(const int* a, const int* b, int* c, size_t n) noexcept;

void sum_loop(const int* a, const int* b, int* c, size_t n) noexcept;

template <class T>
T* allocate_aligned_array(size_t size, size_t alignment) {
  assert(size > 0);

  T* res = nullptr;

#ifdef _WIN32
#error "Windows is not implemented"
#else
  if (posix_memalign((void**)&res, alignment, size * sizeof(T)) != 0) {
    throw std::runtime_error("Memory allocation failed");
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