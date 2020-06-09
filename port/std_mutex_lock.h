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

#ifndef DARWINN_PORT_STD_MUTEX_LOCK_H_
#define DARWINN_PORT_STD_MUTEX_LOCK_H_

#include <mutex>  // NOLINT

#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {

// A wrapper around std::mutex lockers to enable thread annotations. The
// constructor takes a pointer to a mutex, which resembles MutexLock interface.
template<typename T>
class SCOPED_LOCKABLE AnnotatedStdMutexLock : public T {
 public:
  explicit AnnotatedStdMutexLock(std::mutex* mu) EXCLUSIVE_LOCK_FUNCTION(mu)
      : T(*mu) {}

  // This class is neither copyable nor movable.
  AnnotatedStdMutexLock(const AnnotatedStdMutexLock&) = delete;
  AnnotatedStdMutexLock& operator=(const AnnotatedStdMutexLock&) = delete;

  ~AnnotatedStdMutexLock() UNLOCK_FUNCTION() = default;
};

// Intended to be used as a direct replacement of ReaderMutexLock/MutexLock. The
// mutex is locked when constructed, and unlocked when destructed.
typedef AnnotatedStdMutexLock<std::lock_guard<std::mutex>> StdMutexLock;

// Intended to be used as a direct replacement of ReaderMutexLock/MutexLock only
// when std::condition_variable is used with the mutex. Use StdMutexLock
// otherwise. The mutex is locked when constructed, and unlocked when
// destructed.
typedef AnnotatedStdMutexLock<std::unique_lock<std::mutex>> StdCondMutexLock;

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_STD_MUTEX_LOCK_H_
