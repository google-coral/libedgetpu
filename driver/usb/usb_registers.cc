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

#include "driver/usb/usb_registers.h"

#include "driver/usb/usb_ml_commands.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

util::Status UsbRegisters::Open() {
  return util::UnimplementedError("USB register open without attached device");
}

util::Status UsbRegisters::Open(UsbMlCommands* usb_device) {
  usb_device_ = usb_device;
  return util::Status();  // OK
}

util::Status UsbRegisters::Close() {
  usb_device_ = nullptr;
  return util::Status();  // OK
}

util::Status UsbRegisters::Write(uint64 offset, uint64 value) {
  if (usb_device_) {
    return usb_device_->WriteRegister64(static_cast<uint32>(offset), value);
  }
  return util::FailedPreconditionError(
      "USB register write without attached device");
}

util::StatusOr<uint64> UsbRegisters::Read(uint64 offset) {
  if (usb_device_) {
    return usb_device_->ReadRegister64(static_cast<uint32>(offset));
  }
  return util::FailedPreconditionError(
      "USB register read without attached device");
}

util::Status UsbRegisters::Write32(uint64 offset, uint32 value) {
  if (usb_device_) {
    return usb_device_->WriteRegister32(static_cast<uint32>(offset), value);
  }
  return util::FailedPreconditionError(
      "USB register write32 without attached device");
}

util::StatusOr<uint32> UsbRegisters::Read32(uint64 offset) {
  if (usb_device_) {
    return usb_device_->ReadRegister32(static_cast<uint32>(offset));
  }
  return util::FailedPreconditionError(
      "USB register read32 without attached device");
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
