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

#include "driver/driver_helper.h"

#include <unistd.h>

#include <array>
#include <chrono>              // NOLINT
#include <condition_variable>  // NOLINT
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "api/buffer.h"
#include "api/driver.h"
#include "api/package_reference.h"
#include "api/request.h"
#include "driver/executable_util.h"
#include "driver/package_registry.h"
#include "executable/executable_generated.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

namespace {
// Pattern to be filled into guard areas, and output data buffers.
constexpr std::array<unsigned char, 4> GuardPattern = {0xDE, 0xAD, 0xBE, 0xEF};

// Max consecutive matches to count a part of output buffer as not overwritten.
// The shorter, the easier for a false negative (i.e. falsely claiming an error
// has occurred). The longer, the easier for a false positive (i.e. falsely
// claiming no error exists).
constexpr int kMaxConsecutiveMatch = 8;

template <typename T>
void FillAreaWithKnownPattern(const Buffer& guard_area,
                              const T& guard_pattern) {
  auto pc = const_cast<unsigned char*>(guard_area.ptr());
  auto end = guard_area.ptr() + guard_area.size_bytes();
  for (int i = 0; pc != end; ++pc) {
    *pc = guard_pattern[i];
    i = (++i) % guard_pattern.size();
  }
}

template <typename T>
bool CheckIfAreaIsIntact(const Buffer& guard_area, const T& guard_pattern) {
  auto pc = guard_area.ptr();
  auto end = guard_area.ptr() + guard_area.size_bytes();
  for (int i = 0; pc != end; ++pc) {
    if (*pc != guard_pattern[i]) {
      VLOG(1) << StringPrintf(
          "Buffer offset %ld (%p) has been tainted. 0x%X != 0x%X",
          pc - guard_area.ptr(), pc, *pc, guard_pattern[i]);
      return false;
    }
    i = (++i) % guard_pattern.size();
  }

  return true;
}

template <typename T>
bool CheckIfAreaIsCompletelyOverwritten(const Buffer& output_data,
                                        const T& guard_pattern,
                                        int fail_on_consecutive_match) {
  auto pc = output_data.ptr();
  auto end = output_data.ptr() + output_data.size_bytes();
  int count = 0;
  for (int i = 0; pc != end; ++pc) {
    if (*pc == guard_pattern[i]) {
      ++count;
    } else {
      if (count >= fail_on_consecutive_match) {
        break;
      }
      count = 0;
    }
    i = (++i) % guard_pattern.size();
  }

  if (count >= fail_on_consecutive_match) {
    LOG(WARNING) << StringPrintf(
        "Buffer offset %ld (%p) is probably not overwritten by output "
        "activations. Running length: %d",
        (pc - output_data.ptr()) - count, pc - count, count);

    return false;
  }

  return true;
}

// Converts a buffer to a string.
// Similar to model_compiler_file_StoreToString.
std::string ConvertToString(const Buffer::NamedMap& activations) {
  std::vector<std::string> activation_names;
  activation_names.reserve(activations.size());
  for (const auto& activation : activations) {
    activation_names.push_back(activation.first);
  }
  // Named activation buffers are sorted by name in output.
  std::sort(activation_names.begin(), activation_names.end());

  std::string output;
  for (const auto& name : activation_names) {
    const auto& batched_output = activations.at(name);
    for (const auto& output_batch : batched_output) {
      const auto output_batch_string =
          std::string(reinterpret_cast<const char*>(output_batch.ptr()),
                      output_batch.size_bytes());
      output.insert(output.end(), output_batch_string.begin(),
                    output_batch_string.end());
    }
  }

  return output;
}

Status WriteToFile(const std::string& output_file_name,
                   const std::string& output_content) {
  std::ofstream record_file(output_file_name, std::ios_base::out);

  if (record_file.is_open()) {
    record_file.write(output_content.c_str(), output_content.size());
    record_file.close();

    if (!record_file) {
      return InternalError("Failed writing execution record.");
    }
  } else {
    return InternalError("Failed opening file for dumping output.");
  }

  return OkStatus();
}

// Returns true if the actual output matches with expected on the count for each
// unique byte value. This is used to provide a hint that a data mismatch is
// probably caused by re-layout issues.
bool MatchesWithoutRelayout(const uint8* actual_output,
                            const uint8* expected_output, size_t size) {
  constexpr int kNumPossibleValues = std::numeric_limits<uint8>::max() + 1;
  std::array<size_t, kNumPossibleValues> byte_count_actual_output{},
      byte_count_expected_output{};

  // Count the number of each byte value in the outputs.
  for (size_t i = 0; i < size; i++) {
    ++byte_count_actual_output[actual_output[i]];
    ++byte_count_expected_output[expected_output[i]];
  }

  // Make sure they match.
  for (int i = 0; i < kNumPossibleValues; ++i) {
    if (byte_count_expected_output[i] != byte_count_actual_output[i]) {
      return false;
    }
  }

  return true;
}

}  // namespace

DriverHelper::DriverHelper(std::unique_ptr<api::Driver> driver,
                           int max_pending_requests,
                           bool prefill_output_tensors,
                           size_t guard_area_size_bytes)
    : driver_(std::move(driver)),
      max_pending_requests_(max_pending_requests),
      prefill_output_tensors_(prefill_output_tensors),
      guard_area_size_bytes_(guard_area_size_bytes) {}

bool DriverHelper::IsOpen() const { return driver_->IsOpen(); }

bool DriverHelper::IsError() const { return driver_->IsError(); }

Status DriverHelper::Cancel(std::shared_ptr<api::Request> request) {
  return driver_->Cancel(std::move(request));
}

Status DriverHelper::CancelAllRequests() {
  return driver_->CancelAllRequests();
}

uint64_t DriverHelper::allocation_alignment_bytes() const {
  return driver_->allocation_alignment_bytes();
}

Buffer DriverHelper::MakeBuffer(size_t size_bytes) const {
  return driver_->MakeBuffer(size_bytes);
}

Status DriverHelper::Open(bool debug_mode, bool context_lost) {
  return driver_->Open(debug_mode, context_lost);
}

StatusOr<const api::PackageReference*> DriverHelper::RegisterExecutableFile(
    const std::string& executable_filename) {
  return driver_->RegisterExecutableFile(executable_filename);
}

StatusOr<const api::PackageReference*>
DriverHelper::RegisterExecutableSerialized(
    const std::string& executable_content) {
  return driver_->RegisterExecutableSerialized(executable_content);
}

StatusOr<const api::PackageReference*>
DriverHelper::RegisterExecutableSerialized(const char* executable_content,
                                           size_t length) {
  return driver_->RegisterExecutableSerialized(executable_content, length);
}

Status DriverHelper::UnregisterExecutable(
    const api::PackageReference* executable_ref) {
  return driver_->UnregisterExecutable(executable_ref);
}

StatusOr<std::shared_ptr<api::Request>> DriverHelper::CreateRequest(
    const api::PackageReference* executable_ref) {
  return driver_->CreateRequest(executable_ref);
}

Status DriverHelper::Execute(std::shared_ptr<api::Request> request) {
  return driver_->Execute(request);
}

Status DriverHelper::Execute(
    const std::vector<std::shared_ptr<api::Request>>& requests) {
  return driver_->Execute(requests);
}

Status DriverHelper::Submit(std::shared_ptr<api::Request> request,
                            api::Request::Done done_callback) {
  // Request completion callback.
  // Note that the whole callback functor is cloned into this one, so it's
  // available when done.
  auto start_time = std::chrono::steady_clock::now();
  auto wrapped_done = [this, done_callback, start_time](int id,
                                                        const Status& status) {
    auto roundtrip_time_ms = std::chrono::duration<double, std::milli>(
                                 std::chrono::steady_clock::now() - start_time)
                                 .count();
    VLOG(1) << StringPrintf("Request [%d] complete. Status=%s. Took %f ms.", id,
                            status.ToString().c_str(), roundtrip_time_ms);
    StdMutexLock lock(&mutex_);
    CHECK_GT(pending_requests_, 0);
    --pending_requests_;
    roundtrip_times_ms_.push_back(roundtrip_time_ms);
    cv_.notify_all();

    auto verification_start_time = std::chrono::steady_clock::now();
    done_callback(id, status);
    auto verification_time_ms =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - verification_start_time)
            .count();
    verification_times_ms_.push_back(verification_time_ms);
  };

  VLOG(1) << StringPrintf("Request [%d] submitting.", request->id());
  return driver_->Submit(std::move(request), std::move(wrapped_done));
}

Status DriverHelper::Submit(const std::string& tag,
                            const api::PackageReference* executable_ref,
                            const Buffer::NamedMap& input,
                            const Buffer::NamedMap& output,
                            const Buffer::NamedMap& output_with_guard_areas,
                            api::Request::Done request_done) {
  ASSIGN_OR_RETURN(auto request, CreateRequest(executable_ref));

  // Attach inputs to the request.
  for (auto& named_input : input) {
    for (auto& input_buffer : named_input.second) {
      RETURN_IF_ERROR(request->AddInput(named_input.first, input_buffer));
    }
  }

  // Attach outputs to the request.
  for (auto& named_output : output) {
    for (auto& output_buffer : named_output.second) {
      RETURN_IF_ERROR(request->AddOutput(named_output.first, output_buffer));
    }
  }

  // Increase pending and total requests before submission, so the completion
  // callback can make correct calculations. If batching is enabled, each
  // request holds one batch which is multiple inferences.
  {
    StdCondMutexLock lock(&mutex_);
    if (total_requests_ == 0) {
      first_submit_ = std::chrono::steady_clock::now();
    }
    ++pending_requests_;
    ++total_requests_;
  }

  // Submit.
  VLOG(1) << StringPrintf("Request [%d, %s] submitting.", request->id(),
                          tag.c_str());

  auto submit_status = Submit(request, std::move(request_done));

  {
    StdCondMutexLock lock(&mutex_);

    if (!submit_status.ok()) {
      // Decrease request counters, as submission has failed.
      --pending_requests_;
      --total_requests_;
      return submit_status;
    } else {
      // Waits synchronously, if we reach maximum pending requests.
      while (pending_requests_ >= max_pending_requests_) {
        cv_.wait(lock);
      }
    }
  }

  return Status();  // OK.
}

Status DriverHelper::Submit(const std::string& tag,
                            const api::PackageReference* executable_ref,
                            const Buffer::NamedMap& input,
                            const Buffer::NamedMap& expected_output,
                            const Buffer::NamedMap& output) {
  Buffer::NamedMap no_guard_areas;
  return Submit(tag, executable_ref, /*output_file_name=*/std::string{}, input,
                expected_output, output, no_guard_areas);
}

Status DriverHelper::Submit(const std::string& tag,
                            const api::PackageReference* executable_ref,
                            const std::string& output_file_name,
                            const Buffer::NamedMap& input,
                            const Buffer::NamedMap& expected_output,
                            const Buffer::NamedMap& output,
                            const Buffer::NamedMap& output_with_guard_areas) {
  // Note that all the Buffer::NamedMap instances are cloned into the functor
  // when it's created, and hence they can be used to verify correctness of
  // result when the functor is actually executed. Also note that the Buffer
  // objects used are all "host" buffers with shared_ptr, so a memory block
  // would only be released when the last Buffer instance pointing to that
  // memory block is destructed.
  auto request_done = [this, tag, executable_ref, output,
                       output_with_guard_areas, expected_output,
                       output_file_name](int id, const Status& status) {
    if (!status.ok()) {
      LOG(INFO) << StringPrintf("Request [%d, %s] failed: %s", id, tag.c_str(),
                                status.error_message().c_str());
      return;
    }

    // Compare each output buffer.
    for (const auto& output_name : executable_ref->OutputLayerNames()) {
      for (int i = 0; i < expected_output.at(output_name).size(); ++i) {
        const auto& output_buffer = output.at(output_name)[i];
        const auto& expected_output_buffer = expected_output.at(output_name)[i];

        CHECK_EQ(output_buffer.size_bytes(),
                 expected_output_buffer.size_bytes());

        if (prefill_output_tensors_) {
          CHECK(CheckIfAreaIsCompletelyOverwritten(output_buffer, GuardPattern,
                                                   kMaxConsecutiveMatch));
        }

        if (guard_area_size_bytes_ > 0) {
          CHECK(!output_with_guard_areas.empty());
          for (auto& named_output : output) {
            auto it_with_guard_areas =
                output_with_guard_areas.find(named_output.first);
            if (it_with_guard_areas == output_with_guard_areas.end()) {
              LOG(FATAL) << "Cannot find output [" << named_output.first
                         << "] in guard area info";
            }

            const std::vector<Buffer>& device_outputs = named_output.second;
            const std::vector<Buffer>& device_outputs_with_guard_areas =
                it_with_guard_areas->second;

            CHECK_EQ(device_outputs.size(),
                     device_outputs_with_guard_areas.size());

            for (size_t i = 0; i < device_outputs.size(); ++i) {
              // Check the leading guard area is not touched.
              Buffer leading_guard_area(
                  device_outputs_with_guard_areas[i].ptr(),
                  guard_area_size_bytes_);
              CHECK(CheckIfAreaIsIntact(leading_guard_area, GuardPattern))
                  << "Output [" << named_output.first << "][" << i
                  << "]. Leading guard area has been tainted";

              // Check the trailing guard area is not touched.
              // memcmp is going to work as well, but having a separate buffer
              // and call to verify is slightly more flexible regarding size
              // and pattern.
              Buffer trailing_guard_area(
                  device_outputs_with_guard_areas[i].ptr() +
                      guard_area_size_bytes_ + device_outputs[i].size_bytes(),
                  guard_area_size_bytes_);
              CHECK(CheckIfAreaIsIntact(trailing_guard_area, GuardPattern))
                  << "Output [" << named_output.first << "][" << i
                  << "]. Trailing guard area has been tainted";
            }
          }
        }

        if (memcmp(output_buffer.ptr(), expected_output_buffer.ptr(),
                   expected_output_buffer.size_bytes()) != 0) {
          if (MatchesWithoutRelayout(output_buffer.ptr(),
                                     expected_output_buffer.ptr(),
                                     expected_output_buffer.size_bytes())) {
            LOG(ERROR) << StringPrintf(
                "Mismatched result, but every unique byte value has the same "
                "number of elements in both data sets. "
                "This is probably an error related to re-layout\n");
          }

          for (int element = 0; element < expected_output_buffer.size_bytes();
               ++element) {
            if (output_buffer.ptr()[element] !=
                expected_output_buffer.ptr()[element]) {
              if (!output_file_name.empty()) {
                CHECK_OK(
                    WriteToFile(output_file_name, ConvertToString(output)));
              }

              LOG(FATAL) << StringPrintf(
                  "Mismatched result: output_name = %s, batch = %d, "
                  "size_bytes = %zd.\nFirst mismatched element at %d: %x vs "
                  "%x",
                  output_name.c_str(), i, expected_output_buffer.size_bytes(),
                  element, output_buffer.ptr()[element],
                  expected_output_buffer.ptr()[element]);
            }
          }
        }
      }
    }
    LOG(INFO) << StringPrintf("Request [%d, %s] verified.", id, tag.c_str());
  };

  return Submit(tag, executable_ref, input, output, output_with_guard_areas,
                std::move(request_done));
}

Status DriverHelper::Close(api::Driver::ClosingMode mode) {
  StdCondMutexLock lock(&mutex_);
  while (pending_requests_ > 0) {
    VLOG(5) << StringPrintf("Waiting for %d pending requests.",
                            pending_requests_);
    cv_.wait(lock);
  }

  auto last_submit_complete = std::chrono::steady_clock::now();
  auto diff_millis = std::chrono::duration<double, std::milli>(
                         last_submit_complete - first_submit_)
                         .count();
  LOG(INFO) << StringPrintf(
      "%d requests processed in %.3f ms at a rate of %.3f requests per "
      "second or %.3f ms per request.",
      total_requests_, diff_millis, total_requests_ * 1000.0 / diff_millis,
      diff_millis / total_requests_);
  auto sum_verification_time_ms = std::accumulate(
      verification_times_ms_.begin(), verification_times_ms_.end(), 0.0);
  diff_millis -= sum_verification_time_ms;
  LOG(INFO) << StringPrintf(
      "Total process time excluding verification is %.3f ms at a rate of "
      "%.3f requests per second or %.3f ms per request.",
      diff_millis, total_requests_ * 1000.0 / diff_millis,
      diff_millis / total_requests_);
  LOG(INFO) << StringPrintf(
      "Average inference time (As observed by each request which grows with "
      "the number of pending_requests) : %.3f ms.",
      std::accumulate(roundtrip_times_ms_.begin(), roundtrip_times_ms_.end(),
                      0.0) /
          roundtrip_times_ms_.size());

  return driver_->Close(mode);
}

void DriverHelper::SetFatalErrorCallback(FatalErrorCallback callback) {
  driver_->SetFatalErrorCallback(std::move(callback));
}

void DriverHelper::SetThermalWarningCallback(ThermalWarningCallback callback) {
  driver_->SetThermalWarningCallback(std::move(callback));
}

Status DriverHelper::SetRealtimeMode(bool on) {
  return FailedPreconditionError(
      "This driver does not support real-time mode.");
}

Status DriverHelper::SetExecutableTiming(
    const api::PackageReference* executable, const api::Timing& timing) {
  return FailedPreconditionError(
      "This driver does not support real-time mode.");
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
