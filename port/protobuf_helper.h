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

#ifndef DARWINN_PORT_PROTOBUF_HELPER_H_
#define DARWINN_PORT_PROTOBUF_HELPER_H_

#ifdef PROTOBUF_INTERNAL_IMPL

#include "net/proto2/io/public/coded_stream.h"
#include "net/proto2/io/public/zero_copy_stream_impl.h"
#include "net/proto2/public/map.h"
#include "net/proto2/public/repeated_field.h"
#include "net/proto2/public/text_format.h"

using ::proto2::Map;                       // NOLINT
using ::proto2::RepeatedField;             // NOLINT
using ::proto2::RepeatedPtrField;          // NOLINT
using ::proto2::TextFormat;                // NOLINT
using ::proto2::io::CodedOutputStream;     // NOLINT
using ::proto2::io::FileOutputStream;      // NOLINT
using ::proto2::io::StringOutputStream;    // NOLINT

#else

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/map.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/text_format.h>

using ::google::protobuf::Map;                       // NOLINT
using ::google::protobuf::RepeatedField;             // NOLINT
using ::google::protobuf::RepeatedPtrField;          // NOLINT
using ::google::protobuf::TextFormat;                // NOLINT
using ::google::protobuf::io::CodedOutputStream;     // NOLINT
using ::google::protobuf::io::FileOutputStream;      // NOLINT
using ::google::protobuf::io::StringOutputStream;    // NOLINT

#endif

// Note that CodedOutputStream.SetSerializationDeterministic is not available
// in qt-qpr1-dev branch (but it's available in master, which has more recent
// protobuf sources), due to which the following doesn't build in the qpr
// branch.
#if !defined(DARWINN_PORT_ANDROID_SYSTEM)

#include <string>

namespace platforms {
namespace darwinn {

// Produces deterministic serializations, so that the result can be used for
// comparison and hashing.
template <typename Message>
std::string SerializeProto(const Message& message) {
  std::string result;
  StringOutputStream stream(&result);
  CodedOutputStream output(&stream);
  output.SetSerializationDeterministic(true);
  message.SerializeToCodedStream(&output);
  output.Trim();
  return result;
}

}  // namespace darwinn
}  // namespace platforms

#endif  // !defined(DARWINN_PORT_ANDROID_SYSTEM)

#endif  // DARWINN_PORT_PROTOBUF_HELPER_H_
