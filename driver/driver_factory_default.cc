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

#include <dirent.h>
#include <sys/stat.h>

namespace platforms {
namespace darwinn {
namespace driver {

std::vector<api::Device> DriverProvider::EnumerateByClass(
    const std::string& class_name, const std::string& device_name,
    api::Chip chip, api::Device::Type type) {
  std::vector<api::Device> device_list;
  const std::string class_dir_name = "/sys/class/" + class_name;
  DIR* dir = opendir(class_dir_name.c_str());
  if (dir == nullptr) {
    VLOG(2) << "Failed to open " << class_dir_name << ": " << strerror(errno);
    return device_list;  // empty list
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string entry_name(entry->d_name);
    if (entry_name == "." || entry_name == "..") {
      continue;
    }
    if (entry_name.compare(0, device_name.size(), device_name) != 0) {
      continue;
    }
    const std::string dev_file_name = "/dev/" + entry_name;
    struct stat statbuf;
    int ret = stat(dev_file_name.c_str(), &statbuf);
    if (ret != 0) {
      VLOG(1) << "Failed to stat " << dev_file_name << ": " << strerror(errno);
      continue;
    }
    if (!S_ISCHR(statbuf.st_mode)) {
      LOG(ERROR) << dev_file_name << " is not a character device.";
      continue;
    }
    device_list.push_back({chip, type, dev_file_name});
  }

  closedir(dir);
  return device_list;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
