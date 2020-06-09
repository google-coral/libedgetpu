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

#include "driver/interrupt/wire_interrupt_handler.h"

#include <unistd.h>
#include <mutex>  // NOLINT
#include <string>
#include <thread>  // NOLINT

#include "driver/config/common_csr_helper.h"
#include "driver/config/wire_csr_offsets.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/tracing.h"

namespace platforms {
namespace darwinn {
namespace driver {

constexpr const uint64 kQuiescedRegValue = 0xdeadfeeddeadfeedLL;

WireInterruptHandler::WireInterruptHandler(
    Registers* registers, const config::WireCsrOffsets& wire_csr_offsets,
    int num_wires)
    : registers_(registers),
      wire_csr_offsets_(wire_csr_offsets),
      num_wires_(num_wires) {
  CHECK(registers != nullptr);
  // Only supports 1 wire and 3 wire interrupt as of now.
  CHECK(num_wires_ == 1 || num_wires_ == 3);
  interrupts_.resize(Interrupt::DW_INTERRUPT_COUNT);
}

util::Status WireInterruptHandler::ValidateOpenState(bool open) const {
  if (open_ != open) {
    return util::FailedPreconditionError(
        "Invalid state in WireInterruptHandler.");
  }
  return util::Status();  // OK
}

util::Status WireInterruptHandler::Open() {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*open=*/false));
  open_ = true;

  for (int i = 0; i < DW_INTERRUPT_COUNT; ++i) {
    interrupts_[i] = nullptr;
  }

  return util::Status();  // OK
}

util::Status WireInterruptHandler::Close(bool in_error) {
  // If in error, interrupt handler is already serving fatal error, and mutex is
  // already locked. To avoid deadlock, return immediately.
  if (in_error) {
    return util::Status();  // OK
  }

  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*open=*/true));
  open_ = false;

  for (int i = 0; i < DW_INTERRUPT_COUNT; ++i) {
    interrupts_[i] = nullptr;
  }
  return util::Status();  // OK
}

void WireInterruptHandler::MaskInterrupt(int interrupt_id, bool mask) {
  config::registers::WireIntBitArray bit_array_helper_int_mask(ReadMaskArray());
  switch (interrupt_id) {
    case DW_INTERRUPT_INSTR_QUEUE:
      bit_array_helper_int_mask.set_instruction_queue(mask);
      break;
    case DW_INTERRUPT_SC_HOST_0:
      bit_array_helper_int_mask.set_sc_host_0(mask);
      break;
    case DW_INTERRUPT_SC_HOST_1:
      bit_array_helper_int_mask.set_sc_host_1(mask);
      break;
    case DW_INTERRUPT_SC_HOST_2:
      bit_array_helper_int_mask.set_sc_host_2(mask);
      break;
    case DW_INTERRUPT_SC_HOST_3:
      bit_array_helper_int_mask.set_sc_host_3(mask);
      break;
    case DW_INTERRUPT_FATAL_ERR:
      bit_array_helper_int_mask.set_fatal_err(mask);
      break;
    default:
      LOG(FATAL) << "MaskInterrupt: unhandled interrupt id: " << interrupt_id;
  }
  CHECK_OK(WriteMaskArray(bit_array_helper_int_mask.raw()));
}

void WireInterruptHandler::InvokeInterruptWithMask(int interrupt_id) {
  StdMutexLock lock(&mutex_);
  if (interrupts_[interrupt_id]) {
    // Mask and unmask interrupt due to b/111367622.
    // This is Noronha-specific, but shouldn't hurt for other architectures.
    MaskInterrupt(interrupt_id, true);
    interrupts_[interrupt_id]();
    MaskInterrupt(interrupt_id, false);
  }
}

void WireInterruptHandler::InvokeInterrupt(int interrupt_id) {
  StdMutexLock lock(&mutex_);
  if (interrupts_[interrupt_id]) {
    interrupts_[interrupt_id]();
  }
}

uint64 WireInterruptHandler::ReadPendingBitArray() {
  return registers_->Read(wire_csr_offsets_.wire_int_pending_bit_array)
      .ValueOrDie();
}

uint64 WireInterruptHandler::ReadMaskArray() {
  return registers_->Read(wire_csr_offsets_.wire_int_mask_array).ValueOrDie();
}

util::Status WireInterruptHandler::WriteMaskArray(uint64 value) {
  return registers_->Write(wire_csr_offsets_.wire_int_mask_array, value);
}

void WireInterruptHandler::HandlePlatformSingleWireInterrupt() {
  config::registers::WireIntBitArray bit_array_helper(ReadPendingBitArray());
  config::registers::WireIntBitArray bit_array_helper_int_mask(ReadMaskArray());

  while (bit_array_helper.raw() != 0) {
    if (bit_array_helper.raw() == kQuiescedRegValue) {
      // We re-entered this loop after chip was put in clock gating state,
      // hence nothing to do.
      break;
    }

    if (bit_array_helper.instruction_queue()) {
      InvokeInterrupt(DW_INTERRUPT_INSTR_QUEUE);
      bit_array_helper_int_mask.set_instruction_queue(0);
    }

    if (bit_array_helper.sc_host_0()) {
      InvokeInterrupt(DW_INTERRUPT_SC_HOST_0);
      bit_array_helper_int_mask.set_sc_host_0(0);
    }

    if (bit_array_helper.sc_host_1()) {
      InvokeInterrupt(DW_INTERRUPT_SC_HOST_1);
      bit_array_helper_int_mask.set_sc_host_1(0);
    }

    if (bit_array_helper.sc_host_2()) {
      InvokeInterrupt(DW_INTERRUPT_SC_HOST_2);
      bit_array_helper_int_mask.set_sc_host_2(0);
    }

    if (bit_array_helper.sc_host_3()) {
      InvokeInterrupt(DW_INTERRUPT_SC_HOST_3);
      bit_array_helper_int_mask.set_sc_host_3(0);
    }

    if (bit_array_helper.fatal_err()) {
      InvokeInterrupt(DW_INTERRUPT_FATAL_ERR);
      bit_array_helper_int_mask.set_fatal_err(0);
    }

    if (bit_array_helper.top_level_0() || bit_array_helper.top_level_1() ||
        bit_array_helper.top_level_2() || bit_array_helper.top_level_3()) {
      LOG(WARNING) << "Unsupported top level interrupt raised.";
    }

    if (bit_array_helper.param_queue() || bit_array_helper.input_actv_queue() ||
        bit_array_helper.output_actv_queue()) {
      LOG(WARNING) << "Unsupported queue interrupt raised.";
    }

    // Mask bits are set in kernel-land : unmask interrupts when user-land
    // handler has completed.
    bit_array_helper =
        config::registers::WireIntBitArray(ReadPendingBitArray());

    CHECK_OK(WriteMaskArray(bit_array_helper_int_mask.raw()));
  }
}

void WireInterruptHandler::HandleMsi3WireInterrupt(int wire_id) {
  CHECK_LT(wire_id, num_wires_);

  switch (wire_id) {
    // Scalar core interrupt 0.
    case 0:
      InvokeInterruptWithMask(DW_INTERRUPT_SC_HOST_0);
      break;

    // Instruction queue interrupt.
    case 1:
      InvokeInterruptWithMask(DW_INTERRUPT_INSTR_QUEUE);
      break;

    // Remaining.
    default: {
      config::registers::WireIntBitArray bit_array_helper(
          ReadPendingBitArray());

      while (bit_array_helper.raw() != 0) {
        if (bit_array_helper.raw() == kQuiescedRegValue) {
          // We re-entered this loop after chip was put in clock gating state,
          // hence nothing to do.
          break;
        }

        if (bit_array_helper.sc_host_1()) {
          InvokeInterruptWithMask(DW_INTERRUPT_SC_HOST_1);
        }

        if (bit_array_helper.sc_host_2()) {
          InvokeInterruptWithMask(DW_INTERRUPT_SC_HOST_2);
        }

        if (bit_array_helper.sc_host_3()) {
          InvokeInterruptWithMask(DW_INTERRUPT_SC_HOST_3);
        }

        if (bit_array_helper.fatal_err()) {
          InvokeInterruptWithMask(DW_INTERRUPT_FATAL_ERR);
        }

        if (bit_array_helper.top_level_0() || bit_array_helper.top_level_1() ||
            bit_array_helper.top_level_2() || bit_array_helper.top_level_3()) {
          LOG(WARNING) << "Unsupported top level interrupt raised.";
        }

        if (bit_array_helper.param_queue() ||
            bit_array_helper.input_actv_queue() ||
            bit_array_helper.output_actv_queue()) {
          LOG(WARNING) << "Unsupported queue interrupt raised.";
        }

        // Mask bits are set in kernel-land : unmask interrupts when user-land
        // handler has completed.
        bit_array_helper =
            config::registers::WireIntBitArray(ReadPendingBitArray());
      }

      break;
    }
  }
}

void WireInterruptHandler::InvokeAllPendingInterrupts(int wire_id) {
  if (num_wires_ == 3) {
    return HandleMsi3WireInterrupt(wire_id);
  } else {
    return HandlePlatformSingleWireInterrupt();
  }
}

util::Status WireInterruptHandler::Register(Interrupt interrupt,
                                            Handler handler) {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*open=*/true));

  interrupts_[interrupt] = std::move(handler);
  return util::Status();  // OK;
}

PollingWireInterruptHandler::PollingWireInterruptHandler(
    Registers* registers, const config::WireCsrOffsets& wire_csr_offsets,
    std::function<void()> sleep)
    : WireInterruptHandler(registers, wire_csr_offsets, /*num_wires=*/1),
      sleep_(std::move(sleep)) {}

util::Status PollingWireInterruptHandler::Open() {
  StdMutexLock lock(&mutex_);
  if (enabled_) {
    return util::FailedPreconditionError(
        "Invalid state in WireInterruptHandler.");
  }
  RETURN_IF_ERROR(WireInterruptHandler::Open());
  enabled_ = true;

  std::thread event_thread(&PollingWireInterruptHandler::PollInterrupts, this);
  thread_ = std::move(event_thread);

  return util::Status();  // OK
}

util::Status PollingWireInterruptHandler::Close(bool in_error) {
  {
    StdMutexLock lock(&mutex_);
    if (!enabled_) {
      return util::FailedPreconditionError(
          "Invalid state in WireInterruptHandler.");
    }
    enabled_ = false;
  }

  // Wait for thread to exit.
  thread_.join();
  return WireInterruptHandler::Close(in_error);
}

bool PollingWireInterruptHandler::IsEnabled() const {
  StdMutexLock lock(&mutex_);
  return enabled_;
}

void PollingWireInterruptHandler::PollInterrupts() {
  VLOG(5) << StringPrintf("Interrupt monitor thread enter.");
  TRACE_START_THREAD("PollingWireInterruptHandler");

  do {
    sleep_();
    InvokeAllPendingInterrupts(/*wire_id=*/0);
  } while (IsEnabled());

  VLOG(5) << StringPrintf("Interrupt monitor thread exit.");
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
