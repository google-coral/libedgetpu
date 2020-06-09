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

// Copyright 2010 Google Inc. All Rights Reserved.
//
// Basic integer type definitions for various platforms
//
// This code is compiled directly on many platforms, including client
// platforms like Windows, Mac, and embedded systems.  Before making
// any changes here, make sure that you're not breaking any platforms.

#ifndef DARWINN_PORT_DEFAULT_INTEGRAL_TYPES_H_
#define DARWINN_PORT_DEFAULT_INTEGRAL_TYPES_H_

namespace platforms {
namespace darwinn {

// Standard typedefs
// Signed integer types with width of exactly 8, 16, 32, or 64 bits
// respectively, for use when exact sizes are required.
typedef signed char schar;
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

// NOTE: unsigned types are DANGEROUS in loops and other arithmetical
// places.  Use the signed types unless your variable represents a bit
// pattern (eg a hash value) or you really need the extra bit.  Do NOT
// use 'unsigned' to express "this value should always be positive";
// use assertions for this.

// Unsigned integer types with width of exactly 8, 16, 32, or 64 bits
// respectively, for use when exact sizes are required.
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

// A type to represent a Unicode code-point value. As of Unicode 4.0,
// such values require up to 21 bits.
// (For type-checking on pointers, make this explicitly signed,
// and it should always be the signed version of whatever int32 is.)
typedef signed int char32;

//  A type to represent a natural machine word (for e.g. efficiently
// scanning through memory for checksums or index searching). Don't use
// this for storing normal integers. Ideally this would be just
// unsigned int, but our 64-bit architectures use the LP64 model
// (http://en.wikipedia.org/wiki/64-bit_computing#64-bit_data_models), hence
// their ints are only 32 bits. We want to use the same fundamental
// type on all archs if possible to preserve *printf() compatability.
typedef unsigned long uword_t;

#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
#define GG_LL_FORMAT "ll"  // As in "%lld". Note that "q" is poor form also.
#define GG_LL_FORMAT_W L"ll"

static const uint8 kuint8max = static_cast<uint8>(0xFF);
static const uint16 kuint16max = static_cast<uint16>(0xFFFF);
static const uint32 kuint32max = static_cast<uint32>(0xFFFFFFFF);
static const uint64 kuint64max =
    static_cast<uint64>(GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
static const int8 kint8min = static_cast<int8>(~0x7F);
static const int8 kint8max = static_cast<int8>(0x7F);
static const int16 kint16min = static_cast<int16>(~0x7FFF);
static const int16 kint16max = static_cast<int16>(0x7FFF);
static const int32 kint32min = static_cast<int32>(~0x7FFFFFFF);
static const int32 kint32max = static_cast<int32>(0x7FFFFFFF);
static const int64 kint64min =
    static_cast<int64>(GG_LONGLONG(~0x7FFFFFFFFFFFFFFF));
static const int64 kint64max =
    static_cast<int64>(GG_LONGLONG(0x7FFFFFFFFFFFFFFF));

}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_PORT_DEFAULT_INTEGRAL_TYPES_H_
