/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef DARWINN_PORT_DEFAULT_PORT_FROM_TF_MACROS_H_
#define DARWINN_PORT_DEFAULT_PORT_FROM_TF_MACROS_H_

// When building as part of Android System, many of these macros are
// already available through Android.
#if defined(DARWINN_PORT_ANDROID_SYSTEM)

#include "android-base/macros.h"

// Compiler attributes
#if (defined(__GNUC__) || defined(__APPLE__)) && !defined(SWIG)
// Compiler supports GCC-style attributes
#define ATTRIBUTE_NORETURN __attribute__((noreturn))
#define ATTRIBUTE_NOINLINE __attribute__((noinline))
#define ATTRIBUTE_COLD __attribute__((cold))
#define ATTRIBUTE_WEAK __attribute__((weak))
#define ATTRIBUTE_PACKED __attribute__((packed))
#ifdef ABSL_MUST_USE_RESULT
#undef ABSL_MUST_USE_RESULT
#endif
#define ABSL_MUST_USE_RESULT __attribute__((warn_unused_result))
#define PRINTF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__printf__, string_index, first_to_check)))
#define SCANF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__scanf__, string_index, first_to_check)))
#else
#error "Not intended to be compiled by compilers other than clang/gcc"
#endif

// GCC can be told that a certain branch is not likely to be taken (for
// instance, a CHECK failure), and use that information in static analysis.
// Giving it this information can help it optimize for the common case in
// the absence of better information (ie. -fprofile-arcs).
#if defined(COMPILER_GCC3)
#define PREDICT_FALSE(x) (__builtin_expect(x, 0))
#define PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#else
#define PREDICT_FALSE(x) (x)
#define PREDICT_TRUE(x) (x)
#endif

#else  // !DARWINN_PORT_ANDROID_SYSTEM

// Compiler attributes
#if (defined(__GNUC__) || defined(__APPLE__)) && !defined(SWIG)
// Compiler supports GCC-style attributes
#define ATTRIBUTE_NORETURN __attribute__((noreturn))
#define ATTRIBUTE_NOINLINE __attribute__((noinline))
#define ATTRIBUTE_UNUSED __attribute__((unused))
#define ATTRIBUTE_COLD __attribute__((cold))
#define ATTRIBUTE_WEAK __attribute__((weak))
#define ATTRIBUTE_PACKED __attribute__((packed))
#ifdef ABSL_MUST_USE_RESULT
#undef ABSL_MUST_USE_RESULT
#endif
#define ABSL_MUST_USE_RESULT __attribute__((warn_unused_result))
#define PRINTF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__printf__, string_index, first_to_check)))
#define SCANF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__scanf__, string_index, first_to_check)))
#elif defined(COMPILER_MSVC)
// Non-GCC equivalents
#define ATTRIBUTE_NORETURN __declspec(noreturn)
#define ATTRIBUTE_NOINLINE
#define ATTRIBUTE_UNUSED
#define ATTRIBUTE_COLD
#define ABSL_MUST_USE_RESULT
#define ATTRIBUTE_PACKED
#define PRINTF_ATTRIBUTE(string_index, first_to_check)
#define SCANF_ATTRIBUTE(string_index, first_to_check)
#else
// Non-GCC equivalents
#define ATTRIBUTE_NORETURN
#define ATTRIBUTE_NOINLINE
#define ATTRIBUTE_UNUSED
#define ATTRIBUTE_COLD
#define ATTRIBUTE_WEAK
#define ABSL_MUST_USE_RESULT
#define ATTRIBUTE_PACKED
#define PRINTF_ATTRIBUTE(string_index, first_to_check)
#define SCANF_ATTRIBUTE(string_index, first_to_check)
#endif

// GCC can be told that a certain branch is not likely to be taken (for
// instance, a CHECK failure), and use that information in static analysis.
// Giving it this information can help it optimize for the common case in
// the absence of better information (ie. -fprofile-arcs).
#if defined(COMPILER_GCC3)
#define PREDICT_FALSE(x) (__builtin_expect(x, 0))
#define PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#else
#define PREDICT_FALSE(x) (x)
#define PREDICT_TRUE(x) (x)
#endif

// A macro to disallow the copy constructor and operator= functions
// This is usually placed in the private: declarations for a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

// The ARRAYSIZE(arr) macro returns the # of elements in an array arr.
//
// The expression ARRAYSIZE(a) is a compile-time constant of type
// size_t.
#define ARRAYSIZE(a)            \
  ((sizeof(a) / sizeof(*(a))) / \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L || \
    (defined(_MSC_VER) && _MSC_VER >= 1900)
// Define this to 1 if the code is compiled in C++11 mode; leave it
// undefined otherwise.  Do NOT define it to 0 -- that causes
// '#ifdef LANG_CXX11' to behave differently from '#if LANG_CXX11'.
#define LANG_CXX11 1
#endif

#if defined(__clang__) && defined(LANG_CXX11) && defined(__has_warning)
#if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#define FALLTHROUGH_INTENDED [[clang::fallthrough]]  // NOLINT
#endif
#endif

#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED \
  do {                       \
  } while (0)
#endif

#endif  // !DARWINN_PORT_ANDROID_SYSTEM
#endif  // DARWINN_PORT_DEFAULT_PORT_FROM_TF_MACROS_H_
