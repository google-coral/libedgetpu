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

#ifndef DARWINN_PORT_DEFAULT_MUTEX_H_
#define DARWINN_PORT_DEFAULT_MUTEX_H_

#include <mutex>  //NOLINT

#include "port/default/thread_annotations.h"

namespace platforms {
namespace darwinn {

class LOCKABLE Mutex {
 public:
  Mutex() = default;
  ~Mutex() = default;

  void Lock() EXCLUSIVE_LOCK_FUNCTION() { mutex_.lock(); }

  void Unlock() UNLOCK_FUNCTION() { mutex_.unlock(); }

 private:
  std::mutex mutex_;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_MUTEX_H_
