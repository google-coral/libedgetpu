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
#if defined(DARWINN_PORT_ANDROID_SYSTEM)

#include "port/tracer/darwinn_android_native_trace_macros.h"

// For Android binaries built on google3.
#elif defined(__ANDROID__) && defined(DARWINN_ANDROID_GOOGLE3_TRACE_ENABLED)

#include "port/tracer/darwinn_android_google3_trace_macros.h"

// Perfetto tracing for firmware can be enabled at build time.
// Build the firmware with --define darwinn_firmware_trace_enabled=1
// and build the application with --define darwinn_perfetto_trace_enabled=1.
#elif defined(__ANDROID__) && defined(DARWINN_PERFETTO_TRACE_ENABLED)

#include "port/tracer/darwinn_perfetto_trace_macros.h"

// Web Tracing Framework can be enabled at build time.
// --define=GLOBAL_WTF_ENABLE=1
#elif defined(WTF_ENABLE)

#include "port/tracer/darwinn_wtf_trace_macros.h"

// If --define darwinn_csv_trace_enabled=1 is specified, the trace events could
// be dumped into a CSV file.
#elif defined(DARWINN_CSV_TRACE_ENABLED)

#include "port/tracer/darwinn_csv_trace_macros.h"

// If xprof tracing is enabled at build time: --define=darwinn_xprof_enabled=1
// Add --xprof_end_2_end_upload to the test to upload the xprof trace.
#elif defined(DARWINN_XPROF_ENABLED)

#include "port/tracer/darwinn_xprof_trace_macros.h"

// No tracing for other environments.
#else

// Initializes tracing. Only required for Perfetto tracing.
#define TRACE_INITIALIZE()

// Adds a trace event with the start and end time specified by the life time of
// the created scope object.
// The "CRITICAL" makes it visible even for lab testing. See the PNP_BENCHMARK
// below.
#define TRACE_SCOPE_CRITICAL(name)

// Adds a trace event with the start and end time specified by the life time of
// the created scope object. Appends "REQ_ID_" with `request_id` to the event
// name for perfetto, xprof and csv backends, does not append `request_id` for
// other backends. The "CRITICAL" makes it visible even for lab testing. See the
// PNP_BENCHMARK below.
#define TRACE_SCOPE_REQUEST_ID_CRITICAL(name, request_id)

// Starts a trace event. A uint64 unique ID will be returned which could be used
// to match the trace event in TRACE_SCOPE_CRITICAL_END. `device_paths` are
// added as metadata to xprof, and ignored for other tracing backends.
#define TRACE_SCOPE_CRITICAL_BEGIN(name, device_paths) 0
#define TRACE_SCOPE_CRITICAL_END(id)

// For adding a trace event inside anoter scoped trace event. The newly added
// one has the same nested layer as the outer scope trace event.
// Only supported by our Android native tracing.
#define TRACE_WITHIN_SCOPE(name)

// To mark the start of a thread. Only supported by WTF trace.
#define TRACE_START_THREAD(name)

// Dumps the trace events into a file. Only supported by CSV trace.
#define TRACE_DUMP(output_file)

// Marks the end of the profiling. Only required by Perfetto.
#define TRACE_FINALIZE()

#endif

// Don't trace TRACE_SCOPE macros when PNP_BENCHMARKING is on
#ifdef PNP_BENCHMARKING
#define TRACE_SCOPE(name)
#else
#define TRACE_SCOPE(name) TRACE_SCOPE_CRITICAL(name)
#endif

#endif  // DARWINN_PORT_DEFAULT_SYSTRACE_H_
