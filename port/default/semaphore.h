// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DARWINN_PORT_DEFAULT_SEMAPHORE_H_
#define DARWINN_PORT_DEFAULT_SEMAPHORE_H_

#include <stdint.h>

#include <chrono>              //NOLINT
#include <condition_variable>  //NOLINT
#include <mutex>               //NOLINT

namespace platforms {
namespace darwinn {

namespace internal {

// Base class, not for direct use.
class Semaphore {
 public:
  virtual ~Semaphore() = default;

  bool Take() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (count_ == 0) {
      cv_.wait(lock);
    }
    count_--;

    return true;
  }

  bool Take(uint32_t timeout) {
    // Generally we're running one tick per ms.
    auto timeout_time =
        std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

    std::unique_lock<std::mutex> lock(mutex_);
    while (count_ == 0) {
      if (cv_.wait_until(lock, timeout_time) == std::cv_status::timeout) {
        // Timed-out, check the predicate one last time.
        if (count_ != 0) {
          break;
        }

        return false;
      }
    }
    count_--;

    return true;
  }

  bool Give() {
    mutex_.lock();
    if (count_ < max_count_) {
      count_++;
    }
    mutex_.unlock();
    cv_.notify_one();

    return true;
  }

 protected:
  Semaphore(unsigned int max_count, unsigned int initial_count)
      : count_(initial_count), max_count_(max_count) {}

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
  const int max_count_;
};

}  // namespace internal

// Public classes

class BinarySemaphore : public internal::Semaphore {
 public:
  explicit BinarySemaphore(bool set = false)
      : internal::Semaphore(1, set ? 1 : 0) {}
};

class CountingSemaphore : public internal::Semaphore {
 public:
  CountingSemaphore(unsigned int max_count, unsigned int initial_count)
      : internal::Semaphore(max_count, initial_count) {}
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_SEMAPHORE_H_
