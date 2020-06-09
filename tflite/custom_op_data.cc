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

static const char kKeyVersion[] = "1";
// DEPRECATED (Don't reuse key for something else).
// static const char kKeyChipName[] = "2";
static const char kKeyParameterCachingExecutable[] = "3";
static const char kKeyExecutable[] = "4";
static const char kExecutionPreference[] = "5";

}  // namespace

std::unique_ptr<flexbuffers::Builder> SerializeCustomOpData(
    const CustomOpData& custom_op_data) {
  auto builder = gtl::MakeUnique<flexbuffers::Builder>();
  size_t map_start = builder->StartMap();
  builder->Int(kKeyVersion, custom_op_data.version);
  builder->Key(kKeyExecutable);
  builder->String(custom_op_data.executable.data,
                  custom_op_data.executable.length);
  builder->Int(kExecutionPreference, custom_op_data.execution_preference);
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
  // TODO Remove this check once the field is removed.
  CHECK(flexbuffer_map[kKeyParameterCachingExecutable].IsNull());
  auto custom_op_data = gtl::MakeUnique<CustomOpData>();
  custom_op_data->version = flexbuffer_map[kKeyVersion].AsInt32();
  flexbuffers::String executable = flexbuffer_map[kKeyExecutable].AsString();
  custom_op_data->executable = {executable.c_str(), executable.length()};
  if (!flexbuffer_map[kExecutionPreference].IsNull()) {
    custom_op_data->execution_preference =
        flexbuffer_map[kExecutionPreference].AsInt32();
  }
  return custom_op_data;
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
