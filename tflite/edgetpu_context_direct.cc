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

#include "tflite/edgetpu_context_direct.h"

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "api/driver_factory.h"
#include "api/driver_options_generated.h"
#include "port/logging.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "tflite/custom_op_user_data_direct.h"
#include "tflite/edgetpu_manager_direct.h"

namespace platforms {
namespace darwinn {
namespace tflite {

namespace {

using edgetpu::EdgeTpuContext;

// Set default throttled usb performance based on THROTTLE_EDGE_TPU
// THROTTLE_EDGE_TPU = undefined/0: Max; 1: High; 2: Med; 3: Low; others: High
api::PerformanceExpectation DefaultThrottledUsbPerformance(
    edgetpu::DeviceType device_type) {
  api::PerformanceExpectation performance = api::PerformanceExpectation_Max;
#if defined(THROTTLE_EDGE_TPU) && THROTTLE_EDGE_TPU != 0
  if (device_type == edgetpu::DeviceType::kApexUsb) {
    performance = api::PerformanceExpectation_High;
#if THROTTLE_EDGE_TPU == 2
    performance = api::PerformanceExpectation_Medium;
#elif THROTTLE_EDGE_TPU == 3
    performance = api::PerformanceExpectation_Low;
#endif
  }
#endif
  return performance;
}

// Sets performance exectation in a driver option builder.
Status ParsePerformanceExpectationWithDefaultMax(
    edgetpu::DeviceType device_type,
    const std::unordered_map<std::string, std::string>& options,
    api::DriverOptionsBuilder* driver_option_builder) {
  const auto& it = options.find("Performance");
  api::PerformanceExpectation performance = api::PerformanceExpectation_Max;
  if (it == options.end()) {
    performance = DefaultThrottledUsbPerformance(device_type);
    if (performance != api::PerformanceExpectation_Max) {
      VLOG(2) << "Performance expectation: "
              << api::EnumNamePerformanceExpectation(performance)
              << " when USB connected EdgeTpu is throttled";
    } else {
      VLOG(2) << "Performance expectation: Max (default)";
    }
  } else if (it->second == "Low") {
    VLOG(2) << "Performance expectation: Low";
    performance = api::PerformanceExpectation_Low;
  } else if (it->second == "Medium") {
    VLOG(2) << "Performance expectation: Medium";
    performance = api::PerformanceExpectation_Medium;
  } else if (it->second == "High") {
    VLOG(2) << "Performance expectation: High";
    performance = api::PerformanceExpectation_High;
  } else if (it->second == "Max") {
    performance = DefaultThrottledUsbPerformance(device_type);
    if (performance != api::PerformanceExpectation_Max) {
      VLOG(2) << "Performance expectation level Max is not supported when "
                 "USB connected EdgeTpu is throttled. Drop to "
              << api::EnumNamePerformanceExpectation(performance) << ".";
    } else {
      VLOG(2) << "Performance expectation: Max";
    }
  } else {
    return InvalidArgumentError("Invalid performance setting.");
  }

  driver_option_builder->add_performance_expectation(performance);

  return OkStatus();
}

// Sets USB options in a driver USB option builder.
Status ParseUsbOptions(
    const std::unordered_map<std::string, std::string>& options,
    api::DriverUsbOptionsBuilder* usb_option_builder) {
  // Retrieve USB always DFU settings.
  // Setting this to True would force the driver to perform DFU at driver open.
  {
    bool always_dfu = false;
    const auto& it = options.find("Usb.AlwaysDfu");
    if (it == options.end()) {
      VLOG(2) << "USB always DFU: False (default)";
      always_dfu = false;
    } else if (it->second == "True") {
      VLOG(2) << "USB always DFU: True";
      always_dfu = true;
    } else if (it->second == "False") {
      VLOG(2) << "USB always DFU: False";
      always_dfu = false;
    } else {
      return InvalidArgumentError("Invalid USB setting.");
    }

    usb_option_builder->add_always_dfu(always_dfu);
  }

  // Retrieve USB bulk-in queue length limit settings.
  // Setting this to something large, like 32, would give better performance for
  // models with many output layers.
  {
    int bulk_in_queue_capacity = 0;
    const auto& it = options.find("Usb.MaxBulkInQueueLength");
    if (it == options.end()) {
      VLOG(2) << "USB bulk-in queue capacity: default";
    } else {
      // TODO: change to ABSL SimpleAtoi after it's available.
      std::istringstream ss(it->second);
      ss >> bulk_in_queue_capacity;
      if (ss.fail() || !ss.eof()) {
        return InvalidArgumentError(
            "Converting string argument to integer failed.");
      }

      if (bulk_in_queue_capacity == 0) {
        VLOG(2) << "USB queued bulk-in requests disabled";
        usb_option_builder->add_enable_queued_bulk_in_requests(false);
        usb_option_builder->add_has_enable_queued_bulk_in_requests(true);
      } else if ((bulk_in_queue_capacity < 0) ||
                 (bulk_in_queue_capacity > 256)) {
        return InvalidArgumentError(
            "bulk-in queue capacity must be in [0, 256].");
      } else {
        VLOG(2) << "USB bulk-in queue capacity: " << bulk_in_queue_capacity;
        usb_option_builder->add_bulk_in_queue_capacity(bulk_in_queue_capacity);
        usb_option_builder->add_has_bulk_in_queue_capacity(true);
      }
    }
  }

  return OkStatus();
}

}  // namespace

// EdgeTpuDriverWrapper
const char* EdgeTpuDriverWrapper::STATUS_IS_READY = "IsReady";
const char* EdgeTpuDriverWrapper::STATUS_EXCLUSIVE_OWNERSHIP =
    "ExclusiveOwnership";

EdgeTpuDriverWrapper::EdgeTpuDriverWrapper(
    std::unique_ptr<api::Driver> driver,
    const EdgeTpuManager::DeviceEnumerationRecord& enum_record,
    const EdgeTpuManager::DeviceOptions options, bool exclusive_ownership)
    : use_count_(0),
      is_ready_(true),
      is_exclusively_owned_(exclusive_ownership),
      driver_(std::move(driver)),
      enum_record_(enum_record),
      options_(options) {
  VLOG(4) << "Opening device at " << enum_record_.path;
}

EdgeTpuDriverWrapper::~EdgeTpuDriverWrapper() {
  StdMutexLock lock(&mutex_);

  VLOG(4) << "Closing Edge TPU device at " << enum_record_.path;

  (void)driver_->Close(api::Driver::ClosingMode::kGraceful);
  driver_.reset();
  is_ready_ = false;
}

api::Driver* EdgeTpuDriverWrapper::GetDriver() const {
  StdMutexLock lock(&mutex_);

  return driver_.get();
}

Status EdgeTpuDriverWrapper::InvokeExecutable(TfLiteContext* context,
                                                    TfLiteNode* node) {
  CustomOpUserDataDirect* user_data =
      reinterpret_cast<CustomOpUserDataDirect*>(node->user_data);
  auto executable = user_data->GetExecutable();
  std::shared_ptr<api::Request> request;

  {
    StdMutexLock lock(&mutex_);
    if (!driver_ || !is_ready_) {
      return FailedPreconditionError("Edge TPU is not ready.");
    }
    ASSIGN_OR_RETURN(request, driver_->CreateRequest(executable));
  }

  const auto batches = user_data->GetBatches();
  const absl::flat_hash_map<int, int>& variable_output_destination =
      user_data->GetVariableOutputDestination();
  // Attach inputs to the request.
  for (int i = 0; i < executable->NumInputLayers(); ++i) {
    const auto* input = GetInput(context, node, i);
    const auto single_input_size = executable->InputLayer(i)->ActualSizeBytes();
    if (input->buffer_handle != kTfLiteNullBufferHandle && batches > 1) {
      // TODO: How to handle batches > 1?
      return FailedPreconditionError("Too many batches for dma-buf.");
    }
    for (int batch = 0; batch < batches; ++batch) {
      Buffer input_buffer =
          input->buffer_handle == kTfLiteNullBufferHandle
              ? Buffer(input->data.raw + batch * single_input_size,
                       single_input_size)
              : Buffer(input->buffer_handle, single_input_size, false);
      RETURN_IF_ERROR(
          request->AddInput(executable->InputLayerName(i), input_buffer));
    }
  }

  std::vector<Buffer> output_buffers;
  {
    StdMutexLock lock(&mutex_);
    if (!driver_ || !is_ready_) {
      return FailedPreconditionError("Edge TPU is not ready.");
    }

    // Attach outputs to the request.
    output_buffers.reserve(executable->NumOutputLayers() * batches);
    for (int i = 0; i < executable->NumOutputLayers(); ++i) {
      for (int batch = 0; batch < batches; ++batch) {
        Buffer output_buffer =
            driver_->MakeBuffer(executable->OutputLayer(i)->ActualSizeBytes());
        output_buffers.push_back(output_buffer);
        RETURN_IF_ERROR(
            request->AddOutput(executable->OutputLayerName(i), output_buffer));
      }
    }

    // Submit.
    RETURN_IF_ERROR(driver_->Execute(std::move(request)));
  }

  // Relayout tpu outputs to tflite outputs.
  for (int i = 0; i < executable->NumOutputLayers(); ++i) {
    TfLiteTensor* output = nullptr;
    auto it = variable_output_destination.find(i);
    if (it != variable_output_destination.end()) {
      output = GetInput(context, node, it->second);
    } else {
      output = GetOutput(context, node, i);
    }
    int output_size = output->bytes / batches;
    for (int batch = 0; batch < batches; ++batch) {
      RETURN_IF_ERROR(ReFormatOutputs(
          output, batch * output_size, output_size, executable->OutputLayer(i),
          output_buffers[i * batches + batch].ptr()));
    }
  }

  return OkStatus();
}

const EdgeTpuManager::DeviceEnumerationRecord&
EdgeTpuDriverWrapper::GetDeviceEnumRecord() const {
  StdMutexLock lock(&mutex_);

  return enum_record_;
}

EdgeTpuManager::DeviceOptions EdgeTpuDriverWrapper::GetDeviceOptions() const {
  StdMutexLock lock(&mutex_);

  EdgeTpuManager::DeviceOptions status(options_);
  if (is_ready_) {
    status.insert({STATUS_IS_READY, std::string()});
  }
  if (is_exclusively_owned_) {
    status.insert({STATUS_EXCLUSIVE_OWNERSHIP, std::string()});
  }
  return status;
}

Status EdgeTpuDriverWrapper::AddRef() {
  StdMutexLock lock(&mutex_);

  // TODO: Add check for wrap around.
  ++use_count_;

  return OkStatus();
}

int EdgeTpuDriverWrapper::Release() {
  StdMutexLock lock(&mutex_);

  // TODO: Add check for wrap around.
  --use_count_;

  return use_count_;
}

bool EdgeTpuDriverWrapper::IsReady() const {
  StdMutexLock lock(&mutex_);

  return is_ready_;
}

bool EdgeTpuDriverWrapper::IsExclusivelyOwned() const {
  StdMutexLock lock(&mutex_);

  return is_exclusively_owned_;
}

std::unique_ptr<api::Driver> EdgeTpuDriverWrapper::MakeOpenedDriver(
    edgetpu::DeviceType device_type, const std::string& device_path,
    const std::unordered_map<std::string, std::string>& options) {
  auto factory = api::DriverFactory::GetOrCreate();
  if (!factory) {
    VLOG(1) << "Failed to create driver factory.";
    return nullptr;
  }

  flatbuffers::FlatBufferBuilder flatbuffer_builder;

  // TODO: Remove this empty string.
  // Note that flat buffers require allocation to happen before the object's
  // parent, so this string has to be allocated before the option.
  auto empty_public_key = flatbuffer_builder.CreateString("");

  api::DriverUsbOptionsBuilder usb_option_builder(flatbuffer_builder);
  auto parse_result = ParseUsbOptions(options, &usb_option_builder);
  if (!parse_result.ok()) {
    VLOG(1) << parse_result;
    return nullptr;
  }
  auto usb_option = usb_option_builder.Finish();

  api::DriverOptionsBuilder driver_option_builder(flatbuffer_builder);
  driver_option_builder.add_public_key(empty_public_key);
  driver_option_builder.add_verbosity(-1);

  parse_result = ParsePerformanceExpectationWithDefaultMax(
      device_type, options, &driver_option_builder);
  if (!parse_result.ok()) {
    VLOG(1) << parse_result;
    return nullptr;
  }

  driver_option_builder.add_usb(usb_option);
  api::Chip chip;
  api::Device::Type type;

  int i_type = static_cast<int>(device_type);
  switch (i_type) {
    case static_cast<int>(edgetpu::DeviceType::kApexPci):
      chip = api::Chip::kBeagle;
      type = api::Device::Type::PCI;
      break;
    case static_cast<int>(edgetpu::DeviceType::kApexUsb):
      chip = api::Chip::kBeagle;
      type = api::Device::Type::USB;
      break;
    case static_cast<int>(DeviceTypeExtended::kApexReference):
      chip = api::Chip::kBeagle;
      type = api::Device::Type::REFERENCE;
      break;

    default:
      VLOG(1) << "Unsupported device type.";
      return nullptr;
  }

  flatbuffer_builder.Finish(driver_option_builder.Finish());

  auto driver_option = api::Driver::Options(
      flatbuffer_builder.GetBufferPointer(),
      flatbuffer_builder.GetBufferPointer() + flatbuffer_builder.GetSize());

  auto result = factory->CreateDriver({chip, type, device_path}, driver_option);

  if (!result.ok()) {
    VLOG(1) << StringPrintf("Failed to create driver [%s] at [%s]: ",
                            GetDeviceTypeName(device_type),
                            device_path.c_str()) << result.status().ToString();
    return nullptr;
  }

  auto temp_driver = std::move(result).ValueOrDie();
  auto open_status = temp_driver->Open();

  if (!open_status.ok()) {
    VLOG(1) << StringPrintf("Failed to open device [%s] at [%s]: ",
                            GetDeviceTypeName(device_type),
                            device_path.c_str()) << open_status.ToString();
    return nullptr;
  }

  return temp_driver;
}

const char* EdgeTpuDriverWrapper::GetDeviceTypeName(
    edgetpu::DeviceType device_type) {
  int type = static_cast<int>(device_type);

  switch (type) {
    case static_cast<int>(edgetpu::DeviceType::kApexPci):
      return "Apex (PCIe)";
    case static_cast<int>(edgetpu::DeviceType::kApexUsb):
      return "Apex (USB)";
    case static_cast<int>(DeviceTypeExtended::kApexReference):
      return "Apex (Reference)";
    default:
      // Note that many internal device types do not have external names yet, so
      // they cannot be named here.
      return "Unknown";
  }
}

// EdgeTpuContextDirect

EdgeTpuContextDirect::EdgeTpuContextDirect(EdgeTpuDriverWrapper* driver_wrapper)
    : driver_wrapper_(driver_wrapper) {
  // We're not handling notification sent to TfLiteExternalContext::Reresh
  this->Refresh = nullptr;

  CHECK_OK(driver_wrapper_->AddRef());
}

EdgeTpuContextDirect::~EdgeTpuContextDirect() {
  EdgeTpuManagerDirect::GetSingleton()->ReleaseEdgeTpuContext(driver_wrapper_);
  driver_wrapper_ = nullptr;
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms

namespace edgetpu {

edgetpu::EdgeTpuContext::~EdgeTpuContext() = default;

}  // namespace edgetpu
