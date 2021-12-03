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

#ifndef DARWINN_API_ALLOCATED_BUFFER_H_
#define DARWINN_API_ALLOCATED_BUFFER_H_

#include <functional>
#include <cstddef>

namespace platforms {
namespace darwinn {

// A type for buffer that holds (owns) allocated host memory. This class takes
// ownership of the buffer pointers passed into it, freeing them using the given
// Free function when destroyed.
class AllocatedBuffer {
 public:
  // A type for the callback executed to free the buffer.
  using FreeCallback = std::function<void(void*)>;

  AllocatedBuffer(unsigned char* ptr, size_t size_bytes,
                  FreeCallback free_callback);

  ~AllocatedBuffer();

  // Not copyable or movable
  AllocatedBuffer(const AllocatedBuffer&) = delete;
  AllocatedBuffer& operator=(const AllocatedBuffer&) = delete;

  // Returns const buffer pointer.
  const unsigned char* ptr() const { return ptr_; }

  // Returns buffer pointer.
  unsigned char* ptr() { return ptr_; }

  // Size of this buffer in bytes.
  size_t size_bytes() const { return size_bytes_; }

 private:
  // Points to allocated buffer.
  unsigned char* ptr_;

  // Size of the buffer.
  size_t size_bytes_;

  // Callback executed to free the buffer.
  FreeCallback free_callback_;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_ALLOCATED_BUFFER_H_
