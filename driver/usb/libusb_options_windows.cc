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

#include "driver/usb/libusb_options.h"

namespace platforms {
namespace darwinn {
namespace driver {

int SetLibUsbOptions(libusb_context* context) {
  // The UsbDk backend makes libusb behave the most like the Linux version.
  // Without using UsbDk, manual intervention with Administrator privileges
  // is required to have the Windows USB stack detach the DFU configuration
  // and reattach the fully-configured device.
  auto status = libusb_set_option(context, LIBUSB_OPTION_USE_USBDK);
  return status;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
