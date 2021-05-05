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

#ifndef DARWINN_DRIVER_USB_USB_REGISTERS_H_
#define DARWINN_DRIVER_USB_USB_REGISTERS_H_

#include <memory>

#include "driver/registers/registers.h"
#include "driver/usb/usb_ml_commands.h"
#include "port/status.h"
#include "port/statusor.h"

namespace platforms {
namespace darwinn {
namespace driver {

class UsbRegisters : public Registers {
 public:
  UsbRegisters() = default;
  // This version without argument should never be used. Use the version with
  // pointer to USB deviec instead.
  Status Open() override;
  // Enable the USB register object the actually communicate with the underlying
  // device.
  Status Open(UsbMlCommands* usb_device);
  Status Close() override;

  // Accesses 64-bit registers.
  Status Write(uint64 offset, uint64 value) override;
  StatusOr<uint64> Read(uint64 offset) override;

  // Accesses 32-bit registers.
  Status Write32(uint64 offset, uint32 value) override;
  StatusOr<uint32> Read32(uint64 offset) override;

 private:
  // Underlying device.
  UsbMlCommands* usb_device_{nullptr};
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_USB_REGISTERS_H_
