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

#ifndef DARWINN_API_TENSOR_UTIL_H_
#define DARWINN_API_TENSOR_UTIL_H_

#include <string>
#include <vector>

#include "executable/executable_generated.h"
#include "port/logging.h"
#include "port/ptr_util.h"

namespace platforms {
namespace darwinn {
namespace api {
namespace tensor_util {

// Enum for Tensor shape dimension index.
enum ShapeDimension {
  kBatch = 0,
  kY = 1,
  kX = 2,
  kZ = 3,
  kNumDimensions = 4,
};

// Creates a tensor shape object for the dimension lengths.
TensorShapeT MakeTensorShape(const std::vector<int>& dimensions);

// Create a tensor shape object with range information for each dimension.
TensorShapeT MakeTensorShape(const std::vector<Range>& ranges);

// Returns true if all dimensions have valid index ranges.
bool IsValidShape(const TensorShape& shape);
bool IsValidShape(const TensorShapeT& shape);

// Returns an intersection of two shapes. It will return an invalid shape if
// there is no intersection.
TensorShapeT GetIntersectShape(const TensorShapeT& one,
                               const TensorShapeT& two);

// Returns number of elemens in a tensor shape.
int GetNumElementsInShape(const TensorShape& shape);
int GetNumElementsInShape(const TensorShapeT& shape);

// Returns the length of a shape dimension.
int GetDimensionLength(const TensorShape& shape, int dimension);
int GetDimensionLength(const TensorShapeT& shape, int dimension);

// Returns true if a tensor element specified by the position is included in the
// shape.
bool IsElementInShape(const TensorShape& shape,
                      const std::vector<int>& position);
bool IsElementInShape(const TensorShapeT& shape,
                      const std::vector<int>& position);

// Returns a row-major-packed layout for a tensor shape.
std::unique_ptr<TensorLayoutT> BuildPackedLayout(const TensorShape& shape);
std::unique_ptr<TensorLayoutT> BuildPackedLayout(const TensorShapeT& shape);

// Returns true if all dimensions have valid index ranges.
bool IsValidLayout(const TensorLayout& layout);
bool IsValidLayout(const TensorLayoutT& layout);

// Returns true if the layout has no padding.
bool IsNoPaddingLayout(const TensorLayout& layout);

// Return the memory space size for the layout. This can be different from the
// number of valid elements in the layout due to stride.
int GetLayoutSizeInElements(const TensorLayout& layout);
int GetLayoutSizeInElements(const TensorLayoutT& layout);

// Returns a linear memory index from a tensor position (a list of indexes).
int GetMemoryIndexFromPosition(const TensorLayout& layout,
                               const std::vector<int>& position);
int GetMemoryIndexFromPosition(const TensorLayoutT& layout,
                               const std::vector<int>& position);

// Returns a linear memory index of a tensor's first element in memory.
int GetFirstMemoryIndexForShape(const TensorLayout& layout,
                                const TensorShapeT& shape);

// Returns a linear memory index of a tensor's last element in memory.
int GetLastMemoryIndexForShape(const TensorLayout& layout,
                               const TensorShapeT& shape);

// Returns true if all tensor elements are stored in a contiguous layout.
bool IsShapeInContiguousLayout(const TensorLayout& layout,
                               const TensorShapeT& shape);

// Dumps shape information.
std::string DumpShape(const TensorShape& shape);
std::string DumpShape(const TensorShapeT& shape);

// Dumps layout information.
std::string DumpLayout(const TensorLayout& layout);
std::string DumpLayout(const TensorLayoutT& layout);

}  // namespace tensor_util
}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_TENSOR_UTIL_H_
