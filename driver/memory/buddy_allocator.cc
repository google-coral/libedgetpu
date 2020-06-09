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

#include "driver/memory/buddy_allocator.h"

#include "absl/strings/str_format.h"
#include "driver/memory/address_utilities.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// Number of bits in the address space.
constexpr int kAddressSpaceBits = 64;

// Number of bins accounts for powers of 2 in a 64-bit address space, but does
// not need to include bins for sizes smaller than the page size.
constexpr int kNumBins = kAddressSpaceBits - kHostPageShiftBits;

// Based on:
// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
uint64 RoundUpToNextPowerOfTwo(uint64 x) {
  x--;
  x |= x >> 1;   // handle  2 bit numbers
  x |= x >> 2;   // handle  4 bit numbers
  x |= x >> 4;   // handle  8 bit numbers
  x |= x >> 8;   // handle 16 bit numbers
  x |= x >> 16;  // handle 32 bit numbers
  x |= x >> 32;  // handle 64 bit numbers
  x++;

  return x;
}

// Returns the bin index given an order. The unit of allocation is a host page,
// so the smallest bin (bin 0) is for anything that is <= host page size.
int GetBinFromOrder(int order) {
  CHECK_GE(order, kHostPageShiftBits);
  return order - kHostPageShiftBits;
}

// Returns the order for a given bin. For example, bin 2 is of 4 times the host
// page size. On x86 it is 2^(12+2).
int GetOrderFromBin(int bin) { return bin + kHostPageShiftBits; }

// For a given allocation request size, returns the index to the bin (i.e. for
// indexing block_ ) that size belongs to. This based on:
// https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightModLookup
// Rationale:
// pow(2, i) for 0 <= i < 32 have distinct modulo by 37. We use that property to
// perform fast lookup.
int FindBin(uint64 num_pages) {
  const uint64 nearest_power_of_two = RoundUpToNextPowerOfTwo(num_pages);
  // The trick below only works up to 2^31.
  CHECK_LE(nearest_power_of_two, 1ULL << 31);
  static constexpr int
      kMod37BitPosition[] =  // map a bit value mod 37 to its position
      {32, 0,  1,  26, 2,  23, 27, 0,  3, 16, 24, 30, 28, 11, 0,  13, 4,  7, 17,
       0,  25, 22, 31, 15, 29, 10, 12, 6, 0,  21, 14, 9,  5,  20, 8,  19, 18};
  const int num_zero = kMod37BitPosition[nearest_power_of_two % 37];
  return std::max(GetBinFromOrder(num_zero), 0);
}

// Returns the number of pages required to store the specified size in bytes.
uint64 GetNumPages(uint64 size_bytes) {
  const int num_pages = size_bytes >> kHostPageShiftBits;
  const int spillover_page =
      (size_bytes & ((1 << kHostPageShiftBits) - 1)) ? 1 : 0;
  return num_pages + spillover_page;
}

}  // namespace

BuddyAllocator::BuddyAllocator(uint64 address_space_start,
                               uint64 address_space_size_bytes)
    : address_space_start_(address_space_start),
      free_blocks_(kNumBins),
      allocated_blocks_(kNumBins) {
  uint64 offset = 0;
  // Initialize all bins. In the worst case we'd miss up to kHostPageSize - 1
  // bytes.
  for (int i = kAddressSpaceBits - 1; i >= kHostPageShiftBits; --i) {
    const uint64 mask = (1ULL << i);
    if (address_space_size_bytes & mask) {
      free_blocks_[GetBinFromOrder(i)].insert(offset);
      offset += mask;
    }
  }
}

util::StatusOr<uint64> BuddyAllocator::Allocate(uint64 size_bytes) {
  StdMutexLock lock(&mutex_);
  if (size_bytes == 0) {
    return util::InvalidArgumentError("Cannot allocate 0 bytes.");
  }

  const uint64 num_requested_pages = GetNumPages(size_bytes);
  const int desirable_bin = FindBin(num_requested_pages * kHostPageSize);
  int nearest_bin = desirable_bin;

  // Find the nearest bin that has at least something left.
  while (nearest_bin < free_blocks_.size() &&
         free_blocks_[nearest_bin].empty()) {
    ++nearest_bin;
  }
  if (nearest_bin >= free_blocks_.size()) {
    return util::ResourceExhaustedError(
        absl::StrFormat("Can't allocate for 0x%llx bytes.", size_bytes));
  }

  const auto& block = free_blocks_[nearest_bin].begin();
  const uint64 offset = *block;

  free_blocks_[nearest_bin].erase(block);
  allocated_blocks_[desirable_bin].insert(offset);

  // If nearest bin != desirable bin, insert blocks produced by splitting higher
  // order ones
  for (int i = nearest_bin - 1; i >= desirable_bin; --i) {
    const uint64 split_offset = offset + (1ULL << GetOrderFromBin(i));
    free_blocks_[i].insert(split_offset);
  }

  const uint64 allocated_address = offset + address_space_start_;
  return allocated_address;
}

util::Status BuddyAllocator::Free(uint64 address, uint64 size_bytes) {
  StdMutexLock lock(&mutex_);
  const uint64 num_pages = GetNumPages(size_bytes);
  const int bin = FindBin(num_pages * kHostPageSize);

  const uint64 offset = address - address_space_start_;
  auto allocated_iterator = allocated_blocks_[bin].find(offset);
  if (allocated_iterator == allocated_blocks_[bin].end()) {
    return util::InvalidArgumentError(absl::StrFormat(
        "Allocated block with address 0x%llx and size 0x%llx not found.",
        address, size_bytes));
  }
  allocated_blocks_[bin].erase(allocated_iterator);

  uint64 coalesced_offset = offset;
  for (int buddy_bin = bin; buddy_bin < free_blocks_.size(); ++buddy_bin) {
    // Find nearby block ("buddy") if any.
    const uint64 buddy_offset =
        coalesced_offset ^ (1ULL << GetOrderFromBin(buddy_bin));

    auto buddy_iterator = free_blocks_[buddy_bin].find(buddy_offset);
    if (buddy_iterator != free_blocks_[buddy_bin].end()) {
      // Merging with the buddy at buddy_offset.
      free_blocks_[buddy_bin].erase(buddy_iterator);
      coalesced_offset &= buddy_offset;
    } else {
      // We are done - can't coalesce more. Insert the block to the current bin.
      free_blocks_[buddy_bin].insert(coalesced_offset);
      break;
    }
  }

  return util::Status();  // OK.
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
