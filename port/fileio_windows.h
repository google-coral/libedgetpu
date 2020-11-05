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

#ifndef DARWINN_PORT_FILEIO_WINDOWS_H_
#define DARWINN_PORT_FILEIO_WINDOWS_H_

#include <fcntl.h>

// It's defined in port/default/port_from_tf/macros.h and winnt.h which
// is included through windows.h
// port/macros.h should be included only after windows specific code if
// this define is used.
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#include <windows.h>
#undef ARRAYSIZE
// Work around a redundancy between <windows.h> and port/logging.h (enum ERROR)
#undef ERROR

namespace platforms {
namespace darwinn {

#define INVALID_FD_VALUE INVALID_HANDLE_VALUE

typedef HANDLE FileDescriptor;

// Opens existing file on Windows by parsing linux style RD/WR access mode.
// Doesn't support any access options beyond O_RDONLY/O_WRONLY/O_RDWR
FileDescriptor open(const char* path, int access);

// Closes file handle
void close(FileDescriptor fd);

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_FILEIO_WINDOWS_H_
