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

#include "driver/driver_factory.h"
#include "driver/kernel/gasket_ioctl.h"
#include "port/fileio.h"
#include "port/logging.h"

namespace platforms {
namespace darwinn {
namespace driver {

std::vector<api::Device> DriverProvider::EnumerateByClass(
    const std::string& class_name, const std::string& device_name,
    api::Chip chip, api::Device::Type type) {
  std::vector<api::Device> device_list;

  VLOG(1) << "DriverFactoryWin::EnumerateByClass()... ";

  switch (chip) {
    case api::Chip::kBeagle:
      for (int j = 0; j < APEX_MAX_DEVICES; j++) {
        std::string device_path;
        device_path =
            "\\\\?\\" + std::string(APEX_DEVICE_NAME_BASE) + std::to_string(j);

        FileDescriptor fd = CreateFileW(
            std::wstring(device_path.begin(), device_path.end()).c_str(),
            GENERIC_READ,
            0,     // No sharing - expect gle==ERROR_ACCESS_DENIED if already
                   // opened
            NULL,  // default security attributes
            OPEN_EXISTING,  // disposition
            0,              // file attributes
            NULL);          // do not copy file attributes

        if (fd != INVALID_FD_VALUE) {
          VLOG(4) << "devpath=" << device_path;
          device_list.push_back(
              {api::Chip::kBeagle, api::Device::Type::PCI, device_path});
          CloseHandle(fd);
        } else {
          DWORD error = GetLastError();
          if (error == ERROR_FILE_NOT_FOUND) break;
          if (error == ERROR_ACCESS_DENIED) {
            // Device is used by another process.
            VLOG(4) << "devpath=" << device_path << " (in use)";
            device_list.push_back(
                {api::Chip::kBeagle, api::Device::Type::PCI, device_path});
          } else {
            VLOG(4) << "devpath=" << device_path << " open failed with "
                    << error;
          }
        }
      }
      break;

    default:
      LOG(FATAL)
          << "EnumerateByClass is not supported on Windows for this device.";
  }

  if (device_list.empty())
    VLOG(5) << "DriverFactoryWin::EnumerateByClass returns empty list.";
  else
    VLOG(5) << "DriverFactoryWin::EnumerateByClass returns listsize(%d)="
            << device_list.size();

  return device_list;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
