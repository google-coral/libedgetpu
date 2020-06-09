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

#include "api/driver_options_helper.h"

#include "api/driver_options_generated.h"

namespace platforms {
namespace darwinn {
namespace api {

constexpr int64 kOperatingFrequency = 1000000LL;
constexpr int64 kHostTpuBps = 1000000000LL;

// Returns driver options for default.
Driver::Options DriverOptionsHelper::Defaults() {
  flatbuffers::FlatBufferBuilder builder;
  auto options_offset = api::CreateDriverOptions(
      builder,
      /*version=*/1,
      /*usb=*/0,
      /*verbosity=*/0,
      /*performance_expectation=*/api::PerformanceExpectation_High,
      /*public_key=*/builder.CreateString(""),
      /*watchdog_timeout_ns=*/0,
      /*tpu_frequency_hz=*/kOperatingFrequency,
      /*max_scheduled_work_ns=*/-1,
      /*host_to_tpu_bps=*/kHostTpuBps);
  builder.Finish(options_offset);
  return api::Driver::Options(builder.GetBufferPointer(),
                              builder.GetBufferPointer() + builder.GetSize());
}

// Returns driver options for maximum performance.
Driver::Options DriverOptionsHelper::MaxPerformance() {
  flatbuffers::FlatBufferBuilder builder;
  auto options_offset = api::CreateDriverOptions(
      builder,
      /*version=*/1,
      /*usb=*/0,
      /*verbosity=*/0,
      /*performance_expectation=*/api::PerformanceExpectation_Max,
      /*public_key=*/builder.CreateString(""),
      /*watchdog_timeout_ns=*/0,
      /*tpu_frequency_hz=*/kOperatingFrequency,
      /*max_scheduled_work_ns=*/-1,
      /*host_to_tpu_bps=*/kHostTpuBps);
  builder.Finish(options_offset);
  return api::Driver::Options(builder.GetBufferPointer(),
                              builder.GetBufferPointer() + builder.GetSize());
}


}  // namespace api
}  // namespace darwinn
}  // namespace platforms
