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

#include "tflite/custom_op_data.h"

#include <cstdint>
#include <cstring>
#include <vector>

#include "absl/strings/str_format.h"
#include "port/ptr_util.h"
#include "port/status_macros.h"

namespace platforms {
namespace darwinn {
namespace tflite {

namespace {

// CustomOpData struct will be serialized as a Flexbuffer map with the
// following keys.
// "1" ---> integer (version)
// "2" ---> string (DEPRECATED; chip_name).
// "3" ---> string (DEPRECATED; serialized parameter-caching executable)
// "4" ---> string (serialized executable)
// "5" ---> integer (only used by 1.0)
// "6" ---> vector of integers (optional; enum values for chip versions for each
//     executables. It must be present when there are multiple executables)
// "7" ---> vector of strings (optional; additional serialized executables)

static const char kKeyVersion[] = "1";
// DEPRECATED (Don't reuse key for something else).
// static const char kKeyChipName[] = "2";
static const char kKeyParameterCachingExecutable[] = "3";
static const char kKeyExecutable[] = "4";
static const char kExecutionPreference[] = "5";
static const char kVectorOfChipVersions[] = "6";
static const char kVectorOfRemainingExectuables[] = "7";

}  // namespace

std::unique_ptr<flexbuffers::Builder> SerializeCustomOpData(
    const CustomOpData& custom_op_data) {
  auto builder = gtl::MakeUnique<flexbuffers::Builder>();
  size_t map_start = builder->StartMap();
  builder->Int(kKeyVersion, custom_op_data.version);
  builder->Key(kKeyExecutable);
  builder->String(custom_op_data.executables[0].data,
                  custom_op_data.executables[0].length);
  builder->Int(kExecutionPreference, custom_op_data.execution_preference);

  // Starting new fields in the map for config names
  builder->Vector(kVectorOfChipVersions, [&builder, &custom_op_data]() {
    for (const CustomOpWrappedBuffer& buffer : custom_op_data.executables) {
      builder->Int(static_cast<int>(buffer.chip));
    }
  });

  builder->Vector(kVectorOfRemainingExectuables, [&builder, &custom_op_data]() {
    // Skip the first one, as the first executable is stored under
    // "kKeyExecutable" key to maintain backward compatibility.
    for (int i = 1; i < custom_op_data.executables.size(); ++i) {
      builder->String(custom_op_data.executables[i].data,
                      custom_op_data.executables[i].length);
    }
  });

  builder->EndMap(map_start);
  builder->Finish();
  return builder;
}

std::unique_ptr<CustomOpData> DeserializeCustomOpData(const uint8_t* buffer,
                                                      size_t length) {
  if (!buffer || !length) {
    LOG(ERROR) << "Failed to deserialize into CustomOpData object; "
               << " buffer was " << (buffer ? "non-null" : "null")
               << ", length was " << length << " bytes";
    return nullptr;
  }
  auto flexbuffer_map = flexbuffers::GetRoot(buffer, length).AsMap();
  if (!flexbuffer_map[kKeyParameterCachingExecutable].IsNull()) {
    LOG(WARNING)
        << "Deprecated parameter caching executable field is set, ignoring it.";
  }
  auto custom_op_data = gtl::MakeUnique<CustomOpData>();
  custom_op_data->version = flexbuffer_map[kKeyVersion].AsInt32();
  flexbuffers::String executable = flexbuffer_map[kKeyExecutable].AsString();

  if (flexbuffer_map[kVectorOfChipVersions].IsNull()) {
    // This file was serialized by some older version of this code.
    CustomOpWrappedBuffer wrapped_executable;
    wrapped_executable.data = executable.c_str();
    wrapped_executable.length = executable.length();
    wrapped_executable.chip = api::Chip::kUnknown;
    custom_op_data->executables.push_back(wrapped_executable);
  } else {
    // This file was serialized by current version of this code.
    auto chips = flexbuffer_map[kVectorOfChipVersions].AsVector();

    auto remaining_executables =
        flexbuffer_map[kVectorOfRemainingExectuables].AsVector();

    if (chips.IsTheEmptyVector()) {
      LOG(ERROR) << "Failed to deserialize into CustomOpData object; "
                 << " chip names vector is empty";
      return nullptr;
    }

    // Number of chips names should be one more than number of remaining
    // executables.
    if (chips.size() != remaining_executables.size() + 1) {
      LOG(ERROR) << "Failed to deserialize into CustomOpData object; "
                 << " number of config names: " << chips.size()
                 << ", number of remaining executables: "
                 << remaining_executables.size();
      return nullptr;
    }

    for (int i = 0; i < chips.size(); ++i) {
      if (!chips[i].IsInt()) {
        LOG(ERROR) << "Failed to deserialize into CustomOpData object; "
                   << " the " << i << "-th chip version is not an integer.";
        return nullptr;
      }
    }

    custom_op_data->executables.reserve(chips.size());
    CustomOpWrappedBuffer wrapped_executable;
    wrapped_executable.data = executable.c_str();
    wrapped_executable.length = executable.length();
    wrapped_executable.chip = static_cast<api::Chip>(chips[0].AsInt32());
    custom_op_data->executables.push_back(wrapped_executable);

    for (int i = 1; i < chips.size(); ++i) {
      flexbuffers::String exe_bin = remaining_executables[i - 1].AsString();
      CustomOpWrappedBuffer remaining_wrapped_executable;
      remaining_wrapped_executable.data = exe_bin.c_str();
      remaining_wrapped_executable.length = exe_bin.length();
      remaining_wrapped_executable.chip =
          static_cast<api::Chip>(chips[i].AsInt32());
      custom_op_data->executables.push_back(remaining_wrapped_executable);
    }
  }

  if (!flexbuffer_map[kExecutionPreference].IsNull()) {
    custom_op_data->execution_preference =
        flexbuffer_map[kExecutionPreference].AsInt32();
  }
  return custom_op_data;
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
