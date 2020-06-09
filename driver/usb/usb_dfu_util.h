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

#ifndef DARWINN_DRIVER_USB_USB_DFU_UTIL_H_
#define DARWINN_DRIVER_USB_USB_DFU_UTIL_H_

#include "driver/usb/usb_device_interface.h"
#include "driver/usb/usb_dfu_commands.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Performs DFU on device with specified firmware image.
util::Status UsbUpdateDfuDevice(UsbDfuCommands* dfu_device,
                                UsbDeviceInterface::ConstBuffer firmware_image,
                                bool skip_verify);

// TODO: remove this function, as it's only used by the remote
// interface.
// Tries to perform DFU on all USB devices of the same vendor and
// product ID.
util::Status UsbUpdateAllDfuDevices(UsbManager* usb_manager, uint16_t vendor_id,
                                    uint16_t product_id,
                                    const std::string& firmware_filename,
                                    bool skip_verify);

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_USB_USB_DFU_UTIL_H_
