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

#ifndef DARWINN_DRIVER_MMIO_HOST_QUEUE_H_
#define DARWINN_DRIVER_MMIO_HOST_QUEUE_H_

#include <stddef.h>
#include <functional>
#include <memory>
#include <mutex>  // NOLINT
#include <string>
#include <utility>
#include <vector>

#include "api/buffer.h"
#include "driver/config/chip_structures.h"
#include "driver/config/common_csr_helper.h"
#include "driver/config/queue_csr_offsets.h"
#include "driver/device_buffer.h"
#include "driver/hardware_structures.h"
#include "driver/memory/address_space.h"
#include "driver/memory/dma_direction.h"
#include "driver/mmio/coherent_allocator.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/math_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/statusor.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/thread_annotations.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

// This class provides high level interface to manage host queue.
template <typename Element, typename StatusBlock>
class HostQueue {
 public:
  typedef Element queue_element_type;
  typedef StatusBlock status_block_type;

  static const uint64 kEnableBit = 1;
  static const uint64 kDisableBit = 0;

  HostQueue(const config::QueueCsrOffsets& csr_offsets,
            const config::ChipStructures& chip_structures, Registers* registers,
            std::unique_ptr<CoherentAllocator> coherent_allocator, int size,
            bool single_descriptor_mode);

  // This class is neither copyable nor movable.
  HostQueue(const HostQueue&) = delete;
  HostQueue& operator=(const HostQueue&) = delete;

  virtual ~HostQueue() = default;

  // Open/Close the host queue interface.
  virtual Status Open(AddressSpace* address_space);
  virtual Status Close(bool in_error);
  Status Close() { return Close(/*in_error=*/false); }

  // Enqueue the element into the queue with a callback. Does not block. Returns
  // a failure if Enqueue is called when the queue is full.
  virtual Status Enqueue(const Element& element,
                         std::function<void(uint32)> callback);

  // Enable/Disable interrupts.
  virtual Status EnableInterrupts() {
    return RegisterWrite(csr_offsets_.queue_int_control, kEnableBit);
  }
  virtual Status DisableInterrupts() {
    return RegisterWrite(csr_offsets_.queue_int_control, kDisableBit);
  }

  // Process status block to advance |completed_head_|. For each completed
  // element, invoke registered callback.
  void ProcessStatusBlock() LOCKS_EXCLUDED(queue_mutex_);

  // Process status block only if the host queue is open. This is only needed to
  // work around an interrupt race condition in the Darwinn 1.0 stack.
  // See: https://b.corp.google.com/issues/159997870#comment44
  // TODO: Remove this work-around once the DV team has fully
  // transitioned to the 2.0 stack for testing.
  void ProcessStatusBlockIfOpen() LOCKS_EXCLUDED(open_mutex_, queue_mutex_);

  // Return available space in the queue.
  virtual int GetAvailableSpace() const LOCKS_EXCLUDED(queue_mutex_) {
    StdMutexLock lock(&queue_mutex_);
    return GetAvailableSpaceLocked();
  }

  // Returns the size of the queue.
  int size() const { return size_; }

  // Returns true if "address" is within queue address.
  bool IsQueueAddress(void* address) const {
    return address >= queue_ && address < queue_ + size_;
  }

  // Returns true if "address" is within queue and align with a start address of
  // a queue entry.
  bool IsValidQueueEntry(void *address) const {
    if (!IsQueueAddress(address)) return false;
    return (
        reinterpret_cast<uint8*>(address) - reinterpret_cast<uint8*>(queue_)
        ) % sizeof(Element) == 0;
  }

  // Returns true if "address" corresponds to status block address.
  bool IsStatusBlockAddress(void* address) const {
    return address == status_block_;
  }

 private:
  // Returns an error if |open_| is not in the specified state.
  Status CheckState(bool required) const SHARED_LOCKS_REQUIRED(open_mutex_) {
    if (open_ != required) {
      return FailedPreconditionError("Invalid state in HostQueue.");
    }
    return Status();  // OK
  }

  // Helper method to read register at a given offset.
  StatusOr<uint64> RegisterRead(uint64 offset) LOCKS_EXCLUDED(open_mutex_) {
    {
      StdMutexLock lock(&open_mutex_);
      RETURN_IF_ERROR(CheckState(/*required=*/true));
    }
    return registers_->Read(offset);
  }

  // Helper method to read register at a given offset.
  Status RegisterWrite(uint64 offset, uint64 value)
      LOCKS_EXCLUDED(open_mutex_) {
    {
      StdMutexLock lock(&open_mutex_);
      RETURN_IF_ERROR(CheckState(/*required=*/true));
    }
    return registers_->Write(offset, value);
  }

  // Helper method to map all the device addresses.
  void MapAll() {
    DmaDirection dir = DmaDirection::kBidirectional;
    Buffer host_queue(queue_, size_ * sizeof(Element));
    device_queue_buffer_ =
        address_space_->MapCoherentMemory(host_queue, dir,
                                          MappingTypeHint::kSimple)
            .ValueOrDie();

    VLOG(3) << StringPrintf("Queue base : %p -> 0x%016llx [%lu bytes]", queue_,
                            device_queue_buffer_.device_address(),
                            device_queue_buffer_.size_bytes());

    Buffer host_status_block(status_block_, sizeof(StatusBlock));
    device_status_block_buffer_ =
        address_space_
            ->MapCoherentMemory(host_status_block, dir,
                                MappingTypeHint::kSimple).ValueOrDie();

    VLOG(3) << StringPrintf("Queue status block : %p -> 0x%016llx [%lu bytes]",
                            status_block_,
                            device_status_block_buffer_.device_address(),
                            device_status_block_buffer_.size_bytes());
  }

  // Helper method to unmap all the device addresses.
  Status UnmapAll() {
    RETURN_IF_ERROR(
        address_space_->UnmapCoherentMemory(std::move(device_queue_buffer_)));
    RETURN_IF_ERROR(
        address_space_->UnmapCoherentMemory(
            std::move(device_status_block_buffer_)));
    return OkStatus();
  }

  // Helper method to return available space in the queue. Because this is
  // circular queue, only (|size_| - 1) elements are available even if nothing
  // has been enqueued.
  int GetAvailableSpaceLocked() const SHARED_LOCKS_REQUIRED(queue_mutex_) {
    if (single_descriptor_mode_) {
      return completed_head_ == tail_ ? 1 : 0;
    } else {
      // Equivalent to:
      // (tail_ >= completed_head_) ? (size_ - 1 - (tail_ - completed_head_))
      //                            : (completed_head_ - 1 - tail_);
      return (completed_head_ - tail_ - 1) & (size_ - 1);
    }
  }

  // Guards open state.
  mutable std::mutex open_mutex_;

  // Tracks open state.
  bool open_ GUARDED_BY(open_mutex_){false};

  // If true, only allow one outstanding descriptor at a time.
  const bool single_descriptor_mode_{false};

  // Guards queue state such as |tail_|.
  mutable std::mutex queue_mutex_;

  // Gaurds the state for when callbacks are executing.
  mutable std::mutex callback_mutex_;

  // Variables to control queue.
  int completed_head_ GUARDED_BY(queue_mutex_){0};
  int tail_ GUARDED_BY(queue_mutex_){0};

  // Configuration containing all the offsets related to the host queue.
  const config::QueueCsrOffsets csr_offsets_;

  // Register interface to perform read/write on.
  Registers* const registers_;

  // Coherent Allocator interface to get coherent mem DMA'able by our device.
  std::unique_ptr<CoherentAllocator> coherent_allocator_;

  // Size of the HostQueue with respect to the number of |Element|.
  const int size_;

  // Aligned storage and queue pointer for |Element|.
  Element* queue_{nullptr};

  // Aligned storage and pointer for |StatusBlock|.
  StatusBlock* status_block_{nullptr};

  // Callbacks when the enqueued element is done. Error status from status block
  // is passed as an argument.
  std::vector<std::function<void(uint32)>> callbacks_ GUARDED_BY(queue_mutex_);

  // Device addresses.
  DeviceBuffer device_queue_buffer_;
  DeviceBuffer device_status_block_buffer_;

  // Manages device virtual address space.
  AddressSpace* address_space_{nullptr};
};

template <typename Element, typename StatusBlock>
HostQueue<Element, StatusBlock>::HostQueue(
    const config::QueueCsrOffsets& csr_offsets,
    const config::ChipStructures& chip_structures, Registers* registers,
    std::unique_ptr<CoherentAllocator> coherent_allocator, int size,
    bool single_descriptor_mode)
    : single_descriptor_mode_(single_descriptor_mode),
      csr_offsets_(csr_offsets),
      registers_(registers),
      coherent_allocator_(std::move(coherent_allocator)),
      size_(size),
      callbacks_(size) {
  CHECK(registers != nullptr);
  // |size_| is power of 2.
  CHECK_EQ(size_ & (size_ - 1), 0);
  VLOG(3) << "Starting in "
          << (single_descriptor_mode ? "single descriptor" : "normal")
          << " mode";
}

template <typename Element, typename StatusBlock>
Status HostQueue<Element, StatusBlock>::Open(AddressSpace* address_space) {
  StdMutexLock lock(&open_mutex_);
  RETURN_IF_ERROR(CheckState(/*required=*/false));

  if (address_space_ != nullptr) {
    return InternalError("Address space is already set.");
  }
  if (address_space == nullptr) {
    return InvalidArgumentError("Provided address space is null.");
  }
  address_space_ = address_space;

  // Check for pre-conditions to setup host queue correctly.
  ASSIGN_OR_RETURN(auto descriptor_result,
                   registers_->Read(csr_offsets_.queue_descriptor_size));
  if (descriptor_result != sizeof(Element)) {
    return InternalError("Size of |Element| does not match with the hardware.");
  }

  const size_t q_size =
      kHostPageSize *
      (MathUtil::CeilOfRatio<size_t>((sizeof(Element) * size_), kHostPageSize));
  const size_t sb_size =
      kHostPageSize *
      (MathUtil::CeilOfRatio<size_t>(sizeof(StatusBlock), kHostPageSize));

  RETURN_IF_ERROR(coherent_allocator_->Open());

  ASSIGN_OR_RETURN(Buffer queue_mem, coherent_allocator_->Allocate(q_size));
  ASSIGN_OR_RETURN(Buffer status_block_mem,
                   coherent_allocator_->Allocate(sb_size));

  queue_ = reinterpret_cast<Element*>(queue_mem.ptr());
  status_block_ = reinterpret_cast<StatusBlock*>(status_block_mem.ptr());

  // Allocate device addresses.
  MapAll();

  // Setup queue.
  auto status = registers_->Write(csr_offsets_.queue_base,
                                  device_queue_buffer_.device_address());
  status.Update(
      registers_->Write(csr_offsets_.queue_status_block_base,
                        device_status_block_buffer_.device_address()));
  status.Update(registers_->Write(csr_offsets_.queue_size, size_));
  if (!status.ok()) {
    status.Update(UnmapAll());
    return status;
  }

  // Enable the queue, and wait until it's actually enabled.
  config::registers::QueueControl control;
  control.set_enable(kEnableBit);
  control.set_sb_wr_enable(kEnableBit);
  RETURN_IF_ERROR(registers_->Write(csr_offsets_.queue_control, control.raw()));
  RETURN_IF_ERROR(registers_->Poll(csr_offsets_.queue_status, kEnableBit));

  open_ = true;
  return Status();  // OK
}

template <typename Element, typename StatusBlock>
Status HostQueue<Element, StatusBlock>::Close(bool in_error) {
  StdMutexLock lock(&open_mutex_);
  StdMutexLock callback_lock(&callback_mutex_);
  RETURN_IF_ERROR(CheckState(/*required=*/true));

  // Disable the queue.
  RETURN_IF_ERROR(registers_->Write(csr_offsets_.queue_control, kDisableBit));
  if (!in_error) {
    RETURN_IF_ERROR(registers_->Poll(csr_offsets_.queue_status, 0));
  }

  // Tail is software-write only, and is not reset by the hardware.
  auto status = registers_->Write(csr_offsets_.queue_tail, 0);
  // Reset device addresses.
  status.Update(registers_->Write(csr_offsets_.queue_base, 0));
  status.Update(registers_->Write(csr_offsets_.queue_status_block_base, 0));
  RETURN_IF_ERROR(status);

  // Unmap memory.
  RETURN_IF_ERROR(UnmapAll());

  if (address_space_ == nullptr) {
    return InternalError("Address space is already null.");
  }

  address_space_ = nullptr;
  status_block_ = nullptr;
  queue_ = nullptr;
  completed_head_ = 0;
  tail_ = 0;

  // Release coherent memory block.
  RETURN_IF_ERROR(coherent_allocator_->Close());

  open_ = false;
  return Status();  // OK
}

template <typename Element, typename StatusBlock>
Status HostQueue<Element, StatusBlock>::Enqueue(
    const Element& element, std::function<void(uint32)> callback) {
  TRACE_SCOPE("HostQueue::Enqueue");
  StdMutexLock lock(&queue_mutex_);
  if (GetAvailableSpaceLocked() == 0) {
    return UnavailableError(StringPrintf(
        "No space in the queue, completed_head: %d, tail: %d, size: %d",
        completed_head_, tail_, size_));
  }

  VLOG(3) << "Adding an element to the host queue.";

  queue_[tail_] = element;
  callbacks_[tail_] = std::move(callback);

  ++tail_;
  tail_ &= (size_ - 1);

  RETURN_IF_ERROR(RegisterWrite(csr_offsets_.queue_tail, tail_));
  return Status();  // OK
}

template <typename Element, typename StatusBlock>
void HostQueue<Element, StatusBlock>::ProcessStatusBlock() {
  StdMutexLock callback_lock(&callback_mutex_);
  int completed = 0;

  StatusBlock status_block = *status_block_;
  const int completed_until = status_block.completed_head_pointer;
  const uint32 error_status = status_block.fatal_error;

  std::vector<std::function<void(uint32)>> dones;
  {
    StdMutexLock lock(&queue_mutex_);
    while (completed_head_ != completed_until) {
      ++completed;

      if (callbacks_[completed_head_]) {
        dones.push_back(std::move(callbacks_[completed_head_]));
      }
      ++completed_head_;
      completed_head_ &= (size_ - 1);
    }
    VLOG(3) << "Completed " << completed << " elements.";
  }

  // Clear interrupt pending.
  CHECK_OK(RegisterWrite(csr_offsets_.queue_int_status, 0));

  // Perform callbacks.
  for (const auto& done : dones) {
    done(error_status);
  }
}

template <typename Element, typename StatusBlock>
void HostQueue<Element, StatusBlock>::ProcessStatusBlockIfOpen() {
  StdMutexLock lock(&open_mutex_);
  if (!open_) {
    return;
  }

  // Note: The logic from ProcessStatusBlock must be duplicated here as this
  // does not require the instruction queue interrupt status to be cleared,
  // whereas ProcessStatusBlock does.
  //
  // Other than clearing the instruction queue status, this should be identical
  // to ProcessStatusBlock.
  StdMutexLock callback_lock(&callback_mutex_);
  int completed = 0;

  StatusBlock status_block = *status_block_;
  const int completed_until = status_block.completed_head_pointer;
  const uint32 error_status = status_block.fatal_error;

  std::vector<std::function<void(uint32)>> dones;
  {
    StdMutexLock lock(&queue_mutex_);
    while (completed_head_ != completed_until) {
      ++completed;

      if (callbacks_[completed_head_]) {
        dones.push_back(std::move(callbacks_[completed_head_]));
      }
      ++completed_head_;
      completed_head_ &= (size_ - 1);
    }
    VLOG(3) << "Completed " << completed << " elements.";
  }

  // Perform callbacks.
  for (const auto& done : dones) {
    done(error_status);
  }
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_MMIO_HOST_QUEUE_H_
