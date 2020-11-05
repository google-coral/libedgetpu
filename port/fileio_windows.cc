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

#include <string>

#include "port/errors.h"
#include "port/fileio.h"
#include "port/logging.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {

FileDescriptor open(const char* path, int access) {
  std::string c_path(path);
  std::wstring wc_path(c_path.begin(), c_path.end());

  DWORD win_access = (GENERIC_READ | GENERIC_WRITE);
  FileDescriptor fd = INVALID_FD_VALUE;

  switch (access) {
    case O_RDONLY:
      win_access = GENERIC_READ;
      break;

    case O_WRONLY:
      win_access = GENERIC_WRITE;
      break;

    case O_RDWR:
      win_access = (GENERIC_READ | GENERIC_WRITE);
      break;

    default:
      VLOG(1) << StringPrintf("Device open: access unspecified(=%d)", access);
      break;
  }

  fd = CreateFileW(wc_path.c_str(), win_access,
                   (FILE_SHARE_READ | FILE_SHARE_WRITE),
                   NULL,           // default security attributes
                   OPEN_EXISTING,  // disposition
                   0,              // file attributes
                   NULL);          // do not copy file attributes
  if (fd == INVALID_FD_VALUE) {
    DWORD gle = GetLastError();
    VLOG(1) << StringPrintf("Device open: gle=%d strerror=%s", gle,
                            strerror(errno));
  }

  return fd;
}

void close(FileDescriptor fd) {
  BOOL rc = CloseHandle(fd);
  if (!rc) {
    DWORD gle = GetLastError();
    VLOG(1) << StringPrintf("CloseHandle failed: fd(%p) gle=%d strerror=%s", fd,
                            gle, strerror(errno));
  }
}

}  // namespace darwinn
}  // namespace platforms
