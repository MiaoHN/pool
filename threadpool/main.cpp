#include <unistd.h>

#include <functional>
#include <iostream>

#include "threadpool.hpp"

std::mutex _mutex;

int total = 0;

void add() {
  std::lock_guard<std::mutex> locker(_mutex);
  for (int i = 0; i < 100; ++i) {
    total++;
  }
}

int main(int argc, char const *argv[]) {
  ThreadPool::ThreadPool pool;
  for (int i = 0; i < 100000; ++i) {
    pool.Add(add);
  }
  std::cout << "total: " << total << std::endl;
  return 0;
}
