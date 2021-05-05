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

#ifndef DARWINN_API_DRAM_BUFFER_H_
#define DARWINN_API_DRAM_BUFFER_H_

#include "port/status.h"

namespace platforms {
namespace darwinn {

// Represents a buffer backed by on-chip DRAM.
class DramBuffer {
 public:
  DramBuffer() = default;
  virtual ~DramBuffer() = default;

  // Returns the file descriptor to the DRAM buffer.
  virtual int fd() const = 0;

  // Returns size of the buffer in bytes.
  virtual size_t size_bytes() const = 0;

  // Copies size_bytes() bytes of data from source to the buffer.
  virtual Status ReadFrom(void* source) = 0;

  // Copies size_bytes() bytes ofr data from buffer to the destination address.
  virtual Status WriteTo(void* destination) = 0;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_DRAM_BUFFER_H_
