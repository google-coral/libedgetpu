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

#ifndef DARWINN_PORT_BLOCKING_COUNTER_H_
#define DARWINN_PORT_BLOCKING_COUNTER_H_

#include <condition_variable>  // NOLINT
#include <mutex>               // NOLINT

#include "port/std_mutex_lock.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {

// This class allows a thread to block for a pre-specified number of actions.
// Based on absl implementation, execpt that std::mutex is used as opposed to
// absl::Mutex.
// TODO: Remove this implementation when we fully migrate to absl.
// See: cs/absl/synchronization/blocking_counter.h
class BlockingCounter {
 public:
  explicit BlockingCounter(int initial_count) : count_(initial_count) {}

  BlockingCounter(const BlockingCounter&) = delete;
  BlockingCounter& operator=(const BlockingCounter&) = delete;

  // BlockingCounter::DecrementCount()
  //
  // Decrements the counter's "count" by one, and return "count == 0". This
  // function requires that "count != 0" when it is called.
  //
  // Memory ordering: For any threads X and Y, any action taken by X
  // before it calls `DecrementCount()` is visible to thread Y after
  // Y's call to `DecrementCount()`, provided Y's call returns `true`.
  bool DecrementCount();

  // BlockingCounter::Wait()
  //
  // Blocks until the counter reaches zero. This function may be called at most
  // once. On return, `DecrementCount()` will have been called "initial_count"
  // times and the blocking counter may be destroyed.
  //
  // Memory ordering: For any threads X and Y, any action taken by X
  // before X calls `DecrementCount()` is visible to Y after Y returns
  // from `Wait()`.
  void Wait();

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_BLOCKING_COUNTER_H_
