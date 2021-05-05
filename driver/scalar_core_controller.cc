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

#include "driver/scalar_core_controller.h"

#include <limits>

#include "driver/config/common_csr_helper.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// TODO: This should eventually come from some configurations.
constexpr int kNumInterrupts = 4;

}  // namespace

ScalarCoreController::ScalarCoreController(const config::ChipConfig& config,
                                           Registers* registers)
    : hib_user_csr_offsets_(config.GetHibUserCsrOffsets()),
      registers_([registers]() {
        CHECK(registers != nullptr);
        return registers;
      }()),
      interrupt_controller_(config.GetScalarCoreInterruptCsrOffsets(),
                            registers, kNumInterrupts) {
  interrupt_counts_.resize(kNumInterrupts, 0ULL);
}

Status ScalarCoreController::Open() {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*open=*/false));

  // Sets |interrupt_counts_| to initial CSR values.
  auto read_result = registers_->Read(hib_user_csr_offsets_.sc_host_int_count);
  RETURN_IF_ERROR(read_result.status());

  driver::config::registers::ScHostIntCount helper;
  helper.set_raw(read_result.ValueOrDie());

  for (int i = 0; i < kNumInterrupts; ++i) {
    interrupt_counts_[i] = helper.get_field(i);
  }

  open_ = true;
  return Status();  // OK
}

Status ScalarCoreController::Close() {
  StdMutexLock lock(&mutex_);
  RETURN_IF_ERROR(ValidateOpenState(/*open=*/true));

  open_ = false;
  return Status();  // OK
}

Status ScalarCoreController::EnableInterrupts() {
  return interrupt_controller_.EnableInterrupts();
}

Status ScalarCoreController::DisableInterrupts() {
  return interrupt_controller_.DisableInterrupts();
}

Status ScalarCoreController::ClearInterruptStatus(int id) {
  return interrupt_controller_.ClearInterruptStatus(id);
}

StatusOr<uint64> ScalarCoreController::CheckInterruptCounts(int id) {
  {
    StdMutexLock lock(&mutex_);
    RETURN_IF_ERROR(ValidateOpenState(/*open=*/true));
  }

  auto read_result = registers_->Read(hib_user_csr_offsets_.sc_host_int_count);
  RETURN_IF_ERROR(read_result.status());

  driver::config::registers::ScHostIntCount helper;
  helper.set_raw(read_result.ValueOrDie());

  const uint64 new_count = helper.get_field(id);
  const uint64 current_count = interrupt_counts_[id];
  interrupt_counts_[id] = new_count;

  if (new_count >= current_count) {
    return new_count - current_count;
  }

  const uint64 max_counter =
      helper.mask_field(id, std::numeric_limits<uint64>::max());

  return max_counter - current_count + 1 + new_count;
}

Status ScalarCoreController::ValidateOpenState(bool open) const {
  if (open_ != open) {
    return FailedPreconditionError("Invalid state in ScalarCoreController.");
  }
  return Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
