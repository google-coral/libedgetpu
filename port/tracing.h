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

#ifndef DARWINN_PORT_DEFAULT_SYSTRACE_H_
#define DARWINN_PORT_DEFAULT_SYSTRACE_H_

#include <string>

#define DARWINN_SCOPE_PREFIX "DarwiNN::"

// For Android binaries built on Android.
// We borrow the NNAPI systrace implementation to profile DarwiNN drivers built
// on Android.
#if defined(DARWINN_PORT_ANDROID_SYSTEM)

#include "Tracing.h"

#define TRACE_INITIALIZE()
// Use this only once per function, ideally at the beginning of each scope.
#define TRACE_SCOPE(name) NNTRACE_NAME_1(DARWINN_SCOPE_PREFIX name)

// Use this to add trace markers in a scope that already has a TRACE_SCOPE.
#define TRACE_WITHIN_SCOPE(name) NNTRACE_NAME_SWITCH(DARWINN_SCOPE_PREFIX name)

// Use this when a new thread starts up.
#define TRACE_START_THREAD(name)

#define TRACE_DUMP(output_file)
#define TRACE_FINALIZE()

// For Android binaries built on google3.
// When building on google3, blaze will not be able to link against the atrace
// symbols and we will need to dynamically link to it by ourselves.
#elif defined(__ANDROID__) && defined(DARWINN_ANDROID_GOOGLE3_TRACE_ENABLED)

#include "port/tracer/darwinn_android_tracer.h"

#define TRACE_SCOPE(name) \
  ::platforms::darwinn::DARWINN_ANDROID_TRACE_SCOPE(DARWINN_SCOPE_PREFIX name)

#define TRACE_WITHIN_SCOPE(name) \
  ::platforms::darwinn::DARWINN_ANDROID_TRACE_SCOPE(DARWINN_SCOPE_PREFIX name)

#define TRACE_START_THREAD(name)
#define TRACE_DUMP(output_file)
#define TRACE_FINALIZE()

// Perfetto tracing for firmware can be enabled at build time.
// Build the firmware with --define darwinn_firmware_trace_enabled=1
// and build run_graph_executor with --define darwinn_perfetto_trace_enabled=1.
#elif defined(__ANDROID__) && defined(DARWINN_PERFETTO_TRACE_ENABLED)

#include "port/tracer/darwinn_perfetto_scoped_tracer.h"
#include "port/tracer/darwinn_perfetto_tracer.h"

#define TRACE_INITIALIZE() ::platforms::darwinn::InitializeScopedPerfetto()

#define TRACE_SCOPE(name) \
  PERFTETTO_TRACK_SCOPE(DARWINN_SCOPE_PREFIX name)

#define TRACE_WITHIN_SCOPE(name) \
  PERFTETTO_TRACK_SCOPE(DARWINN_SCOPE_PREFIX name)

#define TRACE_START_THREAD(name)
#define TRACE_DUMP(output_file)

#define TRACE_FINALIZE() ::platforms::darwinn::FinalizePerfetto()

// Web Tracing Framework can be enabled at build time.
// --define=GLOBAL_WTF_ENABLE=1
#elif defined(WTF_ENABLE)

#include "third_party/tracing_framework_bindings_cpp/macros.h"  // IWYU pragma: export

#define TRACE_INITIALIZE()
#define TRACE_SCOPE(name) WTF_SCOPE0(DARWINN_SCOPE_PREFIX name)
#define TRACE_WITHIN_SCOPE(name) WTF_EVENT0(DARWINN_SCOPE_PREFIX name)
#define TRACE_START_THREAD(name) WTF_THREAD_ENABLE(DARWINN_SCOPE_PREFIX name)
#define TRACE_DUMP(output_file)
#define TRACE_FINALIZE()

#elif defined(DARWINN_CSV_TRACE_ENABLED)

#include "port/tracer/darwinn_csv_tracer.h"

#define TRACE_INITIALIZE()
#define TRACE_SCOPE(name) \
  ::platforms::darwinn::DARWINN_CSV_TRACE_SCOPE(DARWINN_SCOPE_PREFIX name)
#define TRACE_WITHIN_SCOPE(name) \
  ::platforms::darwinn::DARWINN_CSV_TRACE_SCOPE(DARWINN_SCOPE_PREFIX name)

#define TRACE_START_THREAD(name)

#define TRACE_DUMP(output_file) \
  ::platforms::darwinn::DarwinnCSVTracer::DumpTrace(output_file)

#define TRACE_FINALIZE()

// If xprof tracing is enabled at build time: --define=darwinn_xprof_enabled=1
// To capture the trace, use perftools/gputools/profiler/xprof.sh.
#elif defined(DARWINN_XPROF_ENABLED)

#include "port/tracer/darwinn_fw_xprof_tracer.h"
#include "tensorflow/core/profiler/lib/traceme.h"

#define _PASTE(x, y) x##y
#define PASTE(x, y) _PASTE(x, y)

#define TRACE_INITIALIZE()
#define TRACE_SCOPE(name)                       \
  tensorflow::profiler::TraceMe PASTE(activity, \
                                      __LINE__)(DARWINN_SCOPE_PREFIX name)
#define TRACE_WITHIN_SCOPE(name) TRACE_SCOPE(name)
#define TRACE_START_THREAD(name) TRACE_SCOPE(name)

#define TRACE_FINALIZE()

// No tracing for other environments.
#else

#define TRACE_INITIALIZE()
#define TRACE_SCOPE(name)
#define TRACE_WITHIN_SCOPE(name)
#define TRACE_START_THREAD(name)
#define TRACE_DUMP(output_file)
#define TRACE_FINALIZE()

#endif

#endif  // DARWINN_PORT_DEFAULT_SYSTRACE_H_
