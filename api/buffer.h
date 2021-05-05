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

#ifndef DARWINN_API_BUFFER_H_
#define DARWINN_API_BUFFER_H_

#include <stddef.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/allocated_buffer.h"
#include "api/dram_buffer.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {

// Abstracts a buffer. Movable and copyable.
// TODO: Consider adding two different variants of this class
// for indicating Const and Mutable variants (like ArraySlice). For now,
// const Buffer, requires that contents of underlying buffer is const.
class Buffer {
 public:
  // Convenience structure for keeping track of named array of Buffers.
  using NamedMap = std::unordered_map<std::string, std::vector<Buffer>>;

  // Default constructor. Defaults to an invalid non-existent buffer.
  Buffer() = default;

  // Constructors for wrapping an existing host buffer.
  Buffer(void* buffer, size_t size_bytes);
  Buffer(const void* buffer, size_t size_bytes);

  // Constructors for wrapping an existing host buffer, and optionally hints
  // the runtime to cache it in on-chip memory outside the DarwiNN core.
  Buffer(unsigned char* buffer, size_t size_bytes);
  Buffer(const unsigned char* buffer, size_t size_bytes);

  // Constructors for wrapping an allocated buffer.
  explicit Buffer(std::shared_ptr<AllocatedBuffer> allocated_buffer);

  // Constructor for wrapping a file descriptor for existing memory.
  // on_device_dram: =true, the allocated memory is on DRAM,
  //                 =false, the allocated memory is mmap-able shared memory.
  Buffer(int fd, size_t size_bytes, bool on_device_dram = false);

  // Constructors for wrapping an on-chip DRAM buffer.
  explicit Buffer(std::shared_ptr<DramBuffer> dram_buffer);

  // This type is copyable, with default implementations.
  Buffer(const Buffer&) = default;
  Buffer& operator=(const Buffer&) = default;

  // This type is movable.
  Buffer(Buffer&& other);
  Buffer& operator=(Buffer&& other);

  // Destructors.
  ~Buffer() = default;

  // Get a slice of this buffer. Note that this does not resize the underlying
  // storage, and the original buffer is still valid. The slice will be of the
  // same type as this buffer. In particular, that means there will be an
  // additional shared_ptr reference to the backing memory for allocated
  // buffers.
  // TODO: File descriptor-based buffers cannot be sliced unless
  // the offset is 0.
  Buffer Slice(size_t offset, size_t length) const;

  // Size of this buffer in bytes.
  size_t size_bytes() const { return size_bytes_; }

  // Returns true if buffer is valid.
  bool IsValid() const { return type_ != Type::kInvalid; }

  // Returns buffer pointer.
  const unsigned char* ptr() const;

  // Returns buffer pointer.
  unsigned char* ptr();

  // Returns true if the buffer is backed by some host memory, may or may not be
  // owned by this Buffer.
  bool IsPtrType() const {
    return type_ == Type::kWrapped || type_ == Type::kAllocated;
  }

  // Returns file descriptor.
  int fd() const;

  // Returns true if the buffer is backed by a file descriptor.
  bool FileDescriptorBacked() const {
    return type_ == Type::kFileDescriptor ||
           type_ == Type::kDram ||
           type_ == Type::kDramWrapped;
  }

  // Returns true if this buffer is backed by a DramBuffer.
  bool IsDramType() const {
    return type_ == Type::kDram || type_ == Type::kDramWrapped;
  }

  // Returns true if the buffer is managed by the runtime,
  // i.e., the buffer does not wrap existing memory allocated
  // outside the runtime.
  bool IsManagedType() const {
    return type_ == Type::kAllocated || type_ == Type::kDram;
  }

  // Returns the underlying DRAM Buffer if this buffer is wrapping one managed
  // by the runtime.
  StatusOr<std::shared_ptr<DramBuffer>> GetDramBuffer();

  // Returns a string representation of the buffer for logging/debugging.
  std::string ToString() const;

  // Equality operators.
  bool operator==(const Buffer& rhs) const;
  bool operator!=(const Buffer& rhs) const;

 private:
  // Type for the buffer.
  enum class Type {
    // Invalid.
    kInvalid = 0,

    // Wraps an existing host process addressable buffer.
    kWrapped = 1,

    // Wraps an allocated host process addressable buffer.
    kAllocated = 2,

    // Wraps an mmap-able file descriptor, possibly from ION.
    kFileDescriptor = 3,

    // Wraps a buffer allocated from on-chip DRAM and managed by the runtime.
    kDram = 4,

    // Wraps an existing, i.e., externally allocated, on-chip DRAM
    // allocated buffer not managed by the runtime.
    kDramWrapped = 5,
  };

  // To allow Buffer::Type being tested in CHECK() et al.
  friend std::ostream& operator<< (std::ostream& stream, const Type& type);

  // Type for the buffer.
  Type type_{Type::kInvalid};

  // Size of the buffer.
  size_t size_bytes_{0};

  // Points to host buffer. Valid when type is kWrapped / kAllocated.
  unsigned char* ptr_{nullptr};

  // Points to allocated buffer. Valid when type is kAllocated.
  std::shared_ptr<AllocatedBuffer> allocated_buffer_;

  // File descriptor. Valid when type is kFileDescriptor, kDram or kDramWrapped.
  // Reset to -1 when moved.
  int file_descriptor_{-1};

  // Points to the DramBuffer. Valid when type is kDram.
  std::shared_ptr<DramBuffer> dram_buffer_;
};

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_BUFFER_H_
