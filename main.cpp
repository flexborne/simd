#include "tasks.h"

#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
  std::srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  std::cout << "SIMD calculated first, so cache is warm for the loop\n";
  //task1(false);
  //task2(false);
  task3(false);
  // great performance boost here: by 3-4 times faster than loop in debug
  // and by 5.7-5.8 times faster in release
  task4(false);

  return 0;
}
