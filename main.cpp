#include "tasks.h"

#include <chrono>


int main(int argc, char** argv) {
  std::srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  task1(false);
  // aligned is much faster actually
  task2(false);

  return 0;
}
