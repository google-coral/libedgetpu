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

/*
 * Apex kernel-userspace interface definitions.
 */
#ifndef __APEX_IOCTL_H__
#define __APEX_IOCTL_H__

#include "driver/kernel/gasket_ioctl.h"

/* Clock Gating ioctl. */
struct apex_gate_clock_ioctl {
  /* Enter or leave clock gated state. */
  uint64_t enable;

  /* If set, enter clock gating state, regardless of custom block's
   * internal idle state.
   */
  uint64_t force_idle;
};

/* Performance expectation ioctl. */
enum apex_performance_expectation {
  APEX_PERFORMANCE_LOW = 0,
  APEX_PERFORMANCE_MED = 1,
  APEX_PERFORMANCE_HIGH = 2,
  APEX_PERFORMANCE_MAX = 3,
};

struct apex_performance_expectation_ioctl {
  /* Expected performance from apex. */
  uint32_t performance;
};

/* Base number for all Apex-common IOCTLs. */
#define APEX_IOCTL_BASE 0x7F

/* Enable/Disable clock gating. */
#define APEX_IOCTL_GATE_CLOCK \
  _IOW(APEX_IOCTL_BASE, 0, struct apex_gate_clock_ioctl)

/* Change performance expectation. */
#define APEX_IOCTL_PERFORMANCE_EXPECTATION \
  _IOW(APEX_IOCTL_BASE, 1, struct apex_performance_expectation_ioctl)

#endif /* __APEX_IOCTL_H__ */
