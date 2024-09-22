#include "tasks.h"

#include "common.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
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

float generate_random_number(int f_min, int f_max) {
  assert(f_max > f_min);
  double f = rand() / static_cast<double>(RAND_MAX);
  return f_min + f * (f_max - f_min);
}

template <class T>
void init_arr(std::span<T> arr, int f_min = -10, int f_max = 10) {
  for (size_t i = 0; i < arr.size(); ++i) {
    arr[i] = generate_random_number(f_min, f_max);
  }
}

constexpr bool equal(float a, float b, float eps = 1e-2) noexcept {
  return std::abs(a - b) < eps;
}

}  // namespace

namespace t1 {

constexpr inline auto N = 100000uz;
using type = int;
using array_t = std::array<type, N>;

auto init_arr() noexcept {
  array_t arr;
  ::init_arr<type>(arr);
  return arr;
};

}  // namespace t1

void task1(bool print_arrays) {
  using namespace t1;

  std::cout << std::format("TASK 1, N = {}\n", N);

  stop_watch sw;

  const array_t arr1 = init_arr();
  const array_t arr2 = init_arr();

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

  std::cout << "Loop: " << duration_loop << " us\n";
  std::cout << "SIMD: " << duration_simd << " us\n";
}

namespace t2 {
constexpr auto N = 10'000'000uz;
}  // namespace t2

void task2(bool print_arrays) {
  using namespace t2;
  using type = int;

  std::cout << std::format("TASK 2, N = {}\n", N);

  stop_watch sw;

  auto* res_simd = allocate_aligned_array<type>(N, simd_alignment);
  std::vector<type> res_loop(N);

  // SIMD in the first place, so cache is warm enough for the loop
  // Even though it's not the best way to measure performance
  auto* arr1 = allocate_aligned_array<type>(N, simd_alignment);
  init_arr<type>({arr1, N});

  auto* arr2 = allocate_aligned_array<type>(N, simd_alignment);
  init_arr<type>({arr2, N});

  sw.start();
  sum_simd(arr1, arr2, res_simd, N);
  const auto duration_simd = sw.duration();

  sw.start();
  sum_loop(arr1, arr2, res_loop.data(), N);
  const auto duration_loop = sw.duration();

  assert(std::ranges::equal(res_loop, std::span{res_simd, N}));

  if (print_arrays) {
    print<type>(res_loop, "Loop res:");
    print<type>(std::span{res_simd, N}, "SIMD res:");
  }

  std::cout << "Loop: " << duration_loop << " us\n";
  std::cout << "SIMD: " << duration_simd << " us\n";

  free_aligned_array(arr1);
  free_aligned_array(arr2);
  free_aligned_array(res_simd);
}

namespace t3 {
constexpr auto N = 1'000'000uz;

using type = float;

auto init_arr() {
  auto* arr = allocate_aligned_array<type>(N, simd_alignment);
  ::init_arr<type>({arr, N}, -1, 1);
  return arr;
};

}  // namespace t3

void task3(bool print_arrays) {
  using namespace t3;

  std::cout << std::format("TASK 3, N = {}\n", N);

  stop_watch sw;

  auto* arr1 = init_arr();
  auto* arr2 = init_arr();
  auto* res_simd = allocate_aligned_array<type>(N, simd_alignment);
  std::vector<type> res_loop(N);

  sw.start();
  sum_simd(arr1, arr2, res_simd, N);
  const auto duration_simd_sum = sw.duration();

  sw.start();
  sum_loop(arr1, arr2, res_loop.data(), N);
  const auto duration_loop_sum = sw.duration();

  assert(std::ranges::equal(res_loop, std::span{res_simd, N}));

  if (print_arrays) {
    print<type>(res_loop, "Loop res:");
    print<type>(std::span{res_simd, N}, "SIMD res:");
  }

  sw.start();
  const auto loop_product = dot_product_loop(arr1, arr2, N);
  const auto duration_loop_product = sw.duration();

  sw.start();
  const auto simd_product = dot_product_simd(arr1, arr2, N);
  const auto duration_simd_product = sw.duration();

  // precision is not the best coz of huge amount of numbers and float type
  // so it's okay, if fails
  assert(equal(loop_product, simd_product, 1.f));

  std::cout << std::format("Products loop vs simd: {} vs {}\n", loop_product,
                           simd_product);

  std::cout << std::format(
      "Durations loop vs simd:\ndot product: {} vs {}, simd faster by: {}, "
      "sum: {} vs {}, simd faster by: {}\n",
      duration_loop_product, duration_simd_product,
      static_cast<double>(duration_loop_product) / duration_simd_product,
      duration_loop_sum, duration_simd_sum,
      static_cast<double>(duration_loop_sum) / duration_simd_sum);

  free_aligned_array(arr1);
  free_aligned_array(arr2);
  free_aligned_array(res_simd);
}

namespace t4 {
constexpr auto M = 1000uz;
constexpr auto N = 1000uz;
constexpr auto K = 1000uz;

using type = float;

auto init_arr(size_t n) {
  auto* arr = allocate_aligned_array<type>(n, simd_alignment);
  ::init_arr<type>({arr, n}, -1, 1);
  return arr;
};

void print(const type* arr, size_t rows, size_t cols, std::string_view msg) {
  std::cout << msg << '\n';
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      std::cout << arr[i * cols + j] << ' ';
    }
    std::cout << '\n';
  }
}
}  // namespace t4

void task4(bool print_arrays) {
  using namespace t4;

  std::cout << std::format("TASK 4, M = {}, N = {}, K = {}\n", M, N, K);

  stop_watch sw;

  auto* A = init_arr(M * N);
  auto* B = init_arr(N * K);
  auto* res_simd = allocate_aligned_array<type>(M * K, simd_alignment);
  std::vector<type> res_loop(M * K);

  sw.start();
  matmul_simd_u(A, B, res_simd, M, N, K);
  const auto duration_simd = sw.duration();

  sw.start();
  matmul_loop(A, B, res_loop.data(), M, N, K);
  const auto duration_loop = sw.duration();

  assert(std::ranges::equal(res_loop, std::span{res_simd, res_loop.size()},
                            [](auto a, auto b) { return equal(a, b); }));

  if (print_arrays) {
    print(res_loop.data(), N, K, "Loop res:");
    std::cout << '\n';
    print(res_simd, N, K, "SIMD res:");
  }

  std::cout << std::format(
      "Durations loop vs simd:\ndot product: {} vs {}, simd faster by: {}\n",
      duration_loop, duration_simd,
      static_cast<double>(duration_loop) / duration_simd);

  free_aligned_array(A);
  free_aligned_array(B);
  free_aligned_array(res_simd);
}

namespace t5 {
constexpr auto n_occurences = 500;
}  // namespace t5

void task5() {
  using namespace t5;

  auto* substr = []() {
    // random <=32(!) bytes string
    constexpr static auto data = "abcdedqwe";
    const auto len = strlen(data);
    char* res = allocate_aligned_array<char>(len, simd_alignment);
    memcpy(res, data, strlen(data));
    res[len] = '\0';
    return res;
  }();
  const auto substr_size = strlen(substr);
  assert(substr_size <= 32);

  char* large_str = allocate_aligned_array<char>(n_occurences * substr_size, simd_alignment);
  for (size_t i = 0; i < n_occurences; ++i) {
    memcpy(large_str + i * substr_size, substr, substr_size);
  }
  large_str[n_occurences * substr_size] = '\0';

  stop_watch sw;

  std::cout << std::format("TASK 5, string = {}, substring = {}\n", large_str,
                           substr);

  sw.start();
  const auto res_simd = count_simd({large_str, n_occurences * substr_size}, substr);
  const auto duration_simd = sw.duration();

  sw.start();
  const auto res_loop = count_loop({large_str, n_occurences * substr_size}, substr);
  const auto duration_loop = sw.duration();

  assert(res_loop == res_simd);
  std::cout << std::format("Res loop vs simd: {} vs {}\n", res_loop, res_simd);

  std::cout << std::format(
      "Durations loop vs simd:\n{} vs {}, simd faster by: {}\n", duration_loop,
      duration_simd, static_cast<double>(duration_loop) / duration_simd);

  free_aligned_array(large_str);
  free_aligned_array(substr);
}