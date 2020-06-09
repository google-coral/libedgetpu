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

#ifndef DARWINN_PORT_SHARED_MUTEX_H_
#define DARWINN_PORT_SHARED_MUTEX_H_

#include <condition_variable>  // NOLINT(build/c++11)
#include <mutex>  // NOLINT(build/c++11)

#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {

// A simple implementation of a reader / writer lock.
//
// This allows concurrent reader lock access, but when a writer lock is
// acquired, all other writers and readers will be blocked till the writer
// finishes.
// These locks are not reentrant.
//
// We created this since some of our third_party build targets do not support
// C++14 yet, which is when shared mutex was added.
//
// This implementation also prevents the problem of writer starving. When a
// writer waits for the lock, no other reader can hold the lock. This allows
// the writer to get the lock in a reasonable time.
//
// It is not recommended to use the locking functions in this class directly.
// Please use the scoped wrappers described later in this file.
//
// Example:
//
// SharedMutex mu;
// mu.ReadLock();
// (some read-only operations...)
// mu.ReadUnlock();
//
// mu.WriteLock();
// (some write operations...)
// mu.WriteUnlock();
class LOCKABLE SharedMutex {
 public:
  SharedMutex()
      : reader_count_(0),
        is_writing_(false) {}

  // Blocks the thread until it acquires the lock in shared mode.
  void ReadLock() SHARED_LOCK_FUNCTION();

  // Releases the read share of this SharedMutex.
  void ReadUnlock() UNLOCK_FUNCTION();

  // Blocks the thread untill it acquires the lock exclusively.
  void WriteLock() EXCLUSIVE_LOCK_FUNCTION();

  // Releases the writer lock.
  void WriteUnlock() UNLOCK_FUNCTION();

 private:
  // Internal mutex for every reader / writer to hold before proceed.
  std::mutex mutex_;

  // Condition variable for every thread to wait on other threads.
  std::condition_variable cond_;

  // Count of current active reader.
  int reader_count_;

  // True if the writer lock is owned by some thread.
  bool is_writing_;
};

// Wrapper for the SharedMutex class, which acquires and releases the lock
// in reader / shared mode via RAII.
//
// Example:
// SharedMutex mu;
// foo() {
//   ReaderMutexLock shared_lock(&mu);
// }
class SCOPED_LOCKABLE ReaderMutexLock {
 public:
  explicit ReaderMutexLock(SharedMutex* mu) SHARED_LOCK_FUNCTION(mu)
      : mu_(mu) {
    mu_->ReadLock();
  }

  // This class is neither copyable nor movable.
  ReaderMutexLock(const ReaderMutexLock&) = delete;
  ReaderMutexLock& operator=(const ReaderMutexLock&) = delete;

  ~ReaderMutexLock() UNLOCK_FUNCTION() {
    mu_->ReadUnlock();
  }

 private:
  SharedMutex * const mu_;
};

// Wrapper for the SharedMutex class, which acquires and releases the lock
// in writer / exclusive mode via RAII.
//
// Example:
// SharedMutex mu;
// foo() {
//   WriterMutexLock exclusive_lock(&mu);
// }
class SCOPED_LOCKABLE WriterMutexLock {
 public:
  explicit WriterMutexLock(SharedMutex* mu) EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    mu_->WriteLock();
  }

  // This class is neither copyable nor movable.
  WriterMutexLock(const WriterMutexLock&) = delete;
  WriterMutexLock& operator=(const WriterMutexLock&) = delete;

  ~WriterMutexLock() UNLOCK_FUNCTION() {
    mu_->WriteUnlock();
  }

 private:
  SharedMutex * const mu_;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_SHARED_MUTEX_H_
