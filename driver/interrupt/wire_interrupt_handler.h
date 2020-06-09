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

#ifndef DARWINN_DRIVER_INTERRUPT_WIRE_INTERRUPT_HANDLER_H_
#define DARWINN_DRIVER_INTERRUPT_WIRE_INTERRUPT_HANDLER_H_

#include <mutex>   // NOLINT
#include <thread>  // NOLINT
#include <vector>

#include "driver/config/wire_csr_offsets.h"
#include "driver/interrupt/interrupt_handler.h"
#include "driver/registers/registers.h"
#include "port/status.h"
#include "port/thread_annotations.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Wire Interrupt handler implementation.
class WireInterruptHandler : public InterruptHandler {
 public:
  // Default close to avoid name hiding.
  using InterruptHandler::Close;

  WireInterruptHandler(Registers* registers,
                       const config::WireCsrOffsets& wire_csr_offsets,
                       int num_wires);
  ~WireInterruptHandler() override = default;

  // This class is neither copyable nor movable.
  WireInterruptHandler(const WireInterruptHandler&) = delete;
  WireInterruptHandler& operator=(const WireInterruptHandler&) = delete;

  util::Status Open() LOCKS_EXCLUDED(mutex_) override;
  util::Status Close(bool in_error) LOCKS_EXCLUDED(mutex_) override;

  util::Status Register(Interrupt interrupt, Handler handler)
      LOCKS_EXCLUDED(mutex_) override;

  // Checks the pending bit array and invoke interrupts.
  virtual void InvokeAllPendingInterrupts(int wire_id);

 private:
  // Invokes the handler for the specified interrupt.
  void InvokeInterrupt(int interrupt_id) LOCKS_EXCLUDED(mutex_);

  // Invokes the handler for the specified interrupt, and masks the interrput
  // during processing.
  void InvokeInterruptWithMask(int interrupt_id) LOCKS_EXCLUDED(mutex_);

  // Masks and unmasks given interrput sources.
  void MaskInterrupt(int interrupt_id, bool mask) SHARED_LOCKS_REQUIRED(mutex_);

  // Performs CSR read access.
  uint64 ReadPendingBitArray();
  uint64 ReadMaskArray();

  // Performs CSR write access.
  util::Status WriteMaskArray(uint64 value);

  // Validates that interrupt handler is |open|.
  util::Status ValidateOpenState(bool open) const SHARED_LOCKS_REQUIRED(mutex_);

  // Handles single wire interrupt on platform devices.
  void HandlePlatformSingleWireInterrupt();

  // Handls 3 wire MSI interrupt.
  void HandleMsi3WireInterrupt(int wire_id);

  // Register access.
  Registers* const registers_;

  // CSR offsets.
  const config::WireCsrOffsets wire_csr_offsets_;

  // Number of wires.
  const int num_wires_;

  // Mutex that guards interrupts_, open_ state.;
  mutable std::mutex mutex_;

  // Tracks open state.
  bool open_ GUARDED_BY(mutex_){false};

  // Registered interrupts.
  std::vector<Handler> interrupts_ GUARDED_BY(mutex_);
};

// Wire Interrupt handler implementation that polls the pending bit array.
class PollingWireInterruptHandler : public WireInterruptHandler {
 public:
  // Default close to avoid name hiding.
  using InterruptHandler::Close;

  PollingWireInterruptHandler(Registers* registers,
                              const config::WireCsrOffsets& wire_csr_offsets,
                              std::function<void()> sleep);
  ~PollingWireInterruptHandler() override = default;

  // This class is neither copyable nor movable.
  PollingWireInterruptHandler(const PollingWireInterruptHandler&) = delete;
  PollingWireInterruptHandler& operator=(const PollingWireInterruptHandler&) =
      delete;

  util::Status Open() LOCKS_EXCLUDED(mutex_) override;
  util::Status Close(bool in_error) LOCKS_EXCLUDED(mutex_) override;

 private:
  // Returns true, if polling is enabled.
  bool IsEnabled() const LOCKS_EXCLUDED(mutex_);

  // Polls and dispatches interrupts.
  void PollInterrupts();

  // Mutex that guards enabled_ state.;
  mutable std::mutex mutex_;

  // Tracks enabled state.
  bool enabled_ GUARDED_BY(mutex_){false};

  // Thread for polling interrupts.
  std::thread thread_;

  // Sleep function.
  std::function<void()> sleep_;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_INTERRUPT_WIRE_INTERRUPT_HANDLER_H_
