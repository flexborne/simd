#include "tasks.h"

#include "common.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <ranges>
#include <span>

namespace {

class stop_watch {
 private:
  using clock = std::chrono::high_resolution_clock;

 private:
  std::chrono::time_point<clock> start_time;

 public:
  void start() noexcept { start_time = clock::now(); }

  auto duration() const noexcept { return (clock::now() - start_time).count(); }
};

template <class T>
void print(std::span<const T> range, std::string_view msg) {
  std::cout << msg << '\n';
  std::ranges::copy(range, std::ostream_iterator<T>(std::cout, " "));
  std::cout << '\n';
}

constexpr inline void init_sum_squared(std::span<int> arr) {
  for (size_t i = 0; i < arr.size(); ++i) {
    arr[i] = i * i;
  }
}

constexpr inline void init_sum_cube(std::span<int> arr) {
  for (size_t i = 0; i < arr.size(); ++i) {
    arr[i] = i * i * i * (1 - 2*(i & 1));
  }
}

constexpr auto simd_alignment = 32uz;

}  // namespace

namespace t1 {

constexpr inline auto N = 100000uz;
using array_t = std::array<int, N>;

// could be constexpr, but compiler does not want to: note: constexpr evaluation hit maximum step limit; possible infinite loop
// does not matter much for the task, so I'll leave it just as const
const array_t arr1 = []() {
  array_t arr = {};
  init_sum_squared(arr);
  return arr;
}();

const array_t arr2 = []() {
  array_t arr = {};
  init_sum_cube(arr);
  return arr;
}();

}  // namespace t1

namespace t2 {
  constexpr auto N = 10000000uz;
}

void task1(bool print_arrays) {
  using namespace t1;

  std::cout << std::format("TASK 1, N = {}\n", N);

  stop_watch sw;

  // SIMD in the first place, so cache is warm enough for the loop
  // Even though it's not the best way to measure performance
  array_t res_simd;
  sw.start();
  sum_simd_u(arr1.data(), arr2.data(), res_simd.data(), N);
  const auto duration_simd = sw.duration();

  array_t res_loop;
  sw.start();
  sum_loop(arr1.data(), arr2.data(), res_loop.data(), N);
  const auto duration_loop = sw.duration();

  assert(std::ranges::equal(res_loop, res_simd));

  if (print_arrays) {
    print<int>(res_loop, "Loop res:");
    print<int>(res_simd, "SIMD res:");
  }

  std::cout << "SIMD calculated first, so cache is warm for the loop\n";
  std::cout << "Loop: " << duration_loop << " us\n";
  std::cout << "SIMD: " << duration_simd << " us\n";
}

void task2(bool print_arrays) {
  using namespace t2;

  std::cout << std::format("TASK 2, N = {}\n", N);

  stop_watch sw;

  auto* res_simd = allocate_aligned_array<int>(N, simd_alignment);
  std::vector<int> res_loop(N);

  // SIMD in the first place, so cache is warm enough for the loop
  // Even though it's not the best way to measure performance
  auto* arr1 = allocate_aligned_array<int>(N, simd_alignment);
  init_sum_squared({arr1, N});

  auto* arr2 = allocate_aligned_array<int>(N, simd_alignment);
  init_sum_cube({arr2, N});

  sw.start();
  sum_simd(arr1, arr2, res_simd, N);
  const auto duration_simd = sw.duration();

  sw.start();
  sum_loop(arr1, arr2, res_loop.data(), N);
  const auto duration_loop = sw.duration();

  assert(std::ranges::equal(res_loop, std::span{res_simd, N}));

  if (print_arrays) {
    print<int>(res_loop, "Loop res:");
    print<int>(std::span{res_simd, N}, "SIMD res:");
  }

  std::cout << "SIMD calculated first, so cache is warm for the loop\n";
  std::cout << "Loop: " << duration_loop << " us\n";
  std::cout << "SIMD: " << duration_simd << " us\n";

  free_aligned_array(arr1);
  free_aligned_array(arr2);
  free_aligned_array(res_simd);
}