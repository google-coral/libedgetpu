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

#include "port/shared_mutex.h"

namespace platforms {
namespace darwinn {

void SharedMutex::ReadLock() {
    std::unique_lock<std::mutex> lock(mutex_);
    // Waits for the write lock to be released.
    cond_.wait(lock, [this]{ return !is_writing_; });
    ++reader_count_;
}

void SharedMutex::ReadUnlock() {
  std::unique_lock<std::mutex> lock(mutex_);
  --reader_count_;
  if (reader_count_ == 0) {
    // Notifies the writer thread after this read. We have to notify all threads
    // because there may be a reader waiting behind the writers, and notify_one
    // may target a reader.
    cond_.notify_all();
  }
}

void SharedMutex::WriteLock() {
  std::unique_lock<std::mutex> lock(mutex_);
  // Waits for any other writer thread to finish.
  cond_.wait(lock, [this]{ return !is_writing_; });
  // Indicates that a writer thread is waiting. This blocks other reader
  // threads to acquire the lock.
  is_writing_ = true;
  // Waits for any reader thread to finish.
  cond_.wait(lock, [this]{ return reader_count_ == 0; });
}

void SharedMutex::WriteUnlock() {
  std::unique_lock<std::mutex> lock(mutex_);
  is_writing_ = false;
  // Notifies all pending reader / writer threads.
  cond_.notify_all();
}

}  // namespace darwinn
}  // namespace platforms
