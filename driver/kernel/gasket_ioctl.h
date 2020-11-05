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

/* Common Gasket device kernel and user space declarations. */
#ifndef __GASKET_IOCTL_H__
#define __GASKET_IOCTL_H__

/* Base number for all Gasket-common IOCTLs */
#define GASKET_IOCTL_BASE 0xDC

#ifdef _WIN32
#include "driver/kernel/windows/windows_gasket_ioctl.inc"
#else  // #ifdef _WIN32
#include <linux/ioctl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#ifndef __KERNEL__
#include <stdint.h>
#endif
#endif  // #ifdef _WIN32 else

#include "driver/kernel/common_gasket_ioctl.inc"

#endif /* __GASKET_IOCTL_H__ */
