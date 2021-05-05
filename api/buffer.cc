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

#include "api/buffer.h"

#include <stddef.h>

#include "api/allocated_buffer.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {

Buffer::Buffer(unsigned char* buffer, size_t size_bytes)
    : type_(Type::kWrapped), size_bytes_(size_bytes), ptr_(buffer) {}

Buffer::Buffer(const unsigned char* buffer, size_t size_bytes)
    : Buffer(const_cast<unsigned char*>(buffer), size_bytes) {}

Buffer::Buffer(void* buffer, size_t size_bytes)
    : Buffer(reinterpret_cast<unsigned char*>(buffer), size_bytes) {}

Buffer::Buffer(const void* buffer, size_t size_bytes)
    : Buffer(const_cast<void*>(buffer), size_bytes) {}

Buffer::Buffer(int fd, size_t size_bytes, bool on_device_dram)
    : type_(on_device_dram ? Type::kDramWrapped : Type::kFileDescriptor),
      size_bytes_(size_bytes),
      file_descriptor_(fd) {}

Buffer::Buffer(std::shared_ptr<AllocatedBuffer> allocated_buffer)
    : type_(Type::kAllocated),
      size_bytes_(allocated_buffer->size_bytes()),
      ptr_(allocated_buffer->ptr()),
      allocated_buffer_(std::move(allocated_buffer)) {}

Buffer::Buffer(std::shared_ptr<DramBuffer> dram_buffer)
    : type_(Type::kDram),
      size_bytes_(dram_buffer->size_bytes()),
      file_descriptor_(dram_buffer->fd()),
      dram_buffer_(std::move(dram_buffer)) {}

bool Buffer::operator==(const Buffer& rhs) const {
  return type_ == rhs.type_ && size_bytes_ == rhs.size_bytes_ &&
         ptr_ == rhs.ptr_ && allocated_buffer_ == rhs.allocated_buffer_;
}

bool Buffer::operator!=(const Buffer& rhs) const { return !(*this == rhs); }

Buffer::Buffer(Buffer&& other)
    : type_(other.type_),
      size_bytes_(other.size_bytes_),
      ptr_(other.ptr_),
      allocated_buffer_(std::move(other.allocated_buffer_)),
      file_descriptor_(other.file_descriptor_),
      dram_buffer_(std::move(other.dram_buffer_)) {
  // Explicitly clear out other.
  other.type_ = Type::kInvalid;
  other.ptr_ = 0;
  other.size_bytes_ = 0;
  other.file_descriptor_ = -1;
  // other.allocated_buffer handled in move above.
}

Buffer& Buffer::operator=(Buffer&& other) {
  if (this != &other) {
    type_ = other.type_;
    size_bytes_ = other.size_bytes_;
    ptr_ = other.ptr_;
    file_descriptor_ = other.file_descriptor_;
    allocated_buffer_ = std::move(other.allocated_buffer_);
    dram_buffer_ = std::move(other.dram_buffer_);

    // Explicitly clear out other.
    other.type_ = Type::kInvalid;
    other.ptr_ = 0;
    other.file_descriptor_ = 0;
    other.size_bytes_ = 0;
    // other.allocated_buffer handled in move above.
  }
  return *this;
}

Buffer Buffer::Slice(size_t offset, size_t length) const {
  CHECK_LE(offset + length, size_bytes_);
  CHECK(!FileDescriptorBacked() || offset == 0);

  Buffer ret = *this;
  ret.ptr_ += offset;
  ret.size_bytes_ = length;
  return ret;
}

const unsigned char* Buffer::ptr() const {
  // FD and DRAM type Buffers need to be mapped before use.
  if (type_ == Type::kFileDescriptor ||
      type_ == Type::kDram ||
      type_ == Type::kDramWrapped) {
    LOG(FATAL) << "Called ptr() on buffer type " << type_;
  }

  return ptr_;
}

unsigned char* Buffer::ptr() {
  // FD and DRAM type Buffers need to be mapped before use.
  if (type_ == Type::kFileDescriptor ||
      type_ == Type::kDram ||
      type_ == Type::kDramWrapped) {
    LOG(FATAL) << "Called ptr() on buffer type " << type_;
  }

  return ptr_;
}

int Buffer::fd() const {
  // Only valid with Type == kFileDescriptor, kDram or kDramWrapped
  if (type_ != Type::kFileDescriptor &&
      type_ != Type::kDram &&
      type_ != Type::kDramWrapped) {
    LOG(FATAL) << "Called fd() on buffer type " << type_;
  }

  return file_descriptor_;
}

StatusOr<std::shared_ptr<DramBuffer>> Buffer::GetDramBuffer() {
  if (type_ != Type::kDram) {
    return FailedPreconditionError(
        StringPrintf("Called GetDramBuffer on a buffer of type %d.", type_));
  }
  return dram_buffer_;
}

std::string Buffer::ToString() const {
  if (FileDescriptorBacked()) {
    return StringPrintf("Buffer(fd=%d)", file_descriptor_);
  } else {
    return StringPrintf("Buffer(ptr=%p)", ptr_);
  }
}

std::ostream& operator<<(std::ostream& stream, const Buffer::Type& type) {
  switch (type) {
    case Buffer::Type::kInvalid:
      return (stream << "kInvalid");
    case Buffer::Type::kWrapped:
      return (stream << "kWrapped");
    case Buffer::Type::kAllocated:
      return (stream << "kAllocated");
    case Buffer::Type::kFileDescriptor:
      return (stream << "kFileDescriptor");
    case Buffer::Type::kDram:
      return (stream << "kDram");
    case Buffer::Type::kDramWrapped:
      return (stream << "kDramWrapped");
  }
}
}  // namespace darwinn
}  // namespace platforms
