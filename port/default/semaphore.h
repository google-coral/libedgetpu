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

#include <cstdint>

#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "thread/fiber/fiber.h"
#include "thread/fiber/select.h"
#include "thread/fiber/semaphore/fifo_semaphore.h"

namespace platforms {
namespace darwinn {

namespace internal {

// Base class, not for direct use.
class Semaphore {
 public:
  virtual ~Semaphore() {
    // Give the underlying semaphore its credits back so that it doesn't CHECK.
    semaphore_.Release(semaphore_.capacity() - semaphore_.current_value());
  }

  bool Take() {
    if (thread::Select({thread::OnCancel(), semaphore_.OnAcquire(1)}) == 0) {
      return false;
    }

    return true;
  }

  // A common::firmware::Status return type would be nicer here, but by keeping
  // it as a bool we can avoid the need for a wrapper class in firmware.
  bool Take(uint32_t timeout) {
    // Generally we're running one tick per ms.
    auto timeout_time = absl::Now() + absl::Milliseconds(timeout);

    int i = thread::SelectUntil(timeout_time,
                                {thread::OnCancel(), semaphore_.OnAcquire(1)});
    switch (i) {
      case -1:
        // timeout
        return false;

      case 0:
        // canceled
        return false;
    }

    return true;
  }

  bool Give() {
    // Grab a mutex to prevent multiple givers from releasing at the same time.
    absl::MutexLock lock(&giver_mutex_);

    bool released = false;
    // Prevent the semaphore from overflowing.
    if (semaphore_.current_value() < semaphore_.capacity()) {
      semaphore_.Release(1);
      released = true;
    }

    return released;
  }

 protected:
  Semaphore(unsigned int max_count, unsigned int initial_count)
      : semaphore_(max_count) {
    // By default the FifoSemaphore starts fully given.
    semaphore_.Acquire(max_count - initial_count);
  }

 private:
  thread::FifoSemaphore semaphore_;
  absl::Mutex giver_mutex_;
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
