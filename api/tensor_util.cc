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

#include "api/tensor_util.h"

#include <vector>

#include "executable/executable_generated.h"
#include "port/logging.h"
#include "port/string_util.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace api {
namespace tensor_util {

TensorShapeT MakeTensorShape(const std::vector<int>& dimensions) {
  CHECK_EQ(dimensions.size(), 5);
  TensorShapeT shape;
  shape.dimension.resize(dimensions.size());
  for (int i = 0; i < dimensions.size(); ++i) {
    shape.dimension[i] = {0, dimensions[i] - 1};
  }
  return shape;
}

TensorShapeT MakeTensorShape(const std::vector<Range>& ranges) {
  CHECK_EQ(ranges.size(), 5);
  TensorShapeT shape;
  shape.dimension.resize(ranges.size());
  for (int i = 0; i < ranges.size(); ++i) {
    shape.dimension[i] = ranges[i];
  }
  return shape;
}

TensorShapeT GetIntersectShape(const TensorShapeT& one,
                               const TensorShapeT& two) {
  CHECK_EQ(one.dimension.size(), two.dimension.size());
  TensorShapeT intersect;
  intersect.dimension.resize(one.dimension.size());
  for (int i = 0; i < one.dimension.size(); ++i) {
    intersect.dimension[i] = {
        std::max(one.dimension[i].start(), two.dimension[i].start()),
        std::min(one.dimension[i].end(), two.dimension[i].end())};
  }
  return intersect;
}

bool IsValidShape(const TensorShape& shape) {
  if (shape.dimension()->size() == 0) {
    return false;
  }

  for (int i = 0; i < shape.dimension()->size(); ++i) {
    if (shape.dimension()->Get(i)->start() > shape.dimension()->Get(i)->end()) {
      return false;
    }
  }
  return true;
}

bool IsValidShape(const TensorShapeT& shape) {
  if (shape.dimension.empty()) {
    return false;
  }

  for (int i = 0; i < shape.dimension.size(); ++i) {
    if (shape.dimension[i].start() > shape.dimension[i].end()) {
      return false;
    }
  }
  return true;
}

int GetNumElementsInShape(const TensorShape& shape) {
  int num_elements = 1;
  for (int i = 0; i < shape.dimension()->size(); ++i) {
    const int length = shape.dimension()->Get(i)->end() -
                       shape.dimension()->Get(i)->start() + 1;
    CHECK_GT(length, 0);
    num_elements *= length;
  }
  return num_elements;
}

int GetNumElementsInShape(const TensorShapeT& shape) {
  int num_elements = 1;
  for (int i = 0; i < shape.dimension.size(); ++i) {
    const int length =
        shape.dimension[i].end() - shape.dimension[i].start() + 1;
    CHECK_GT(length, 0);
    num_elements *= length;
  }
  return num_elements;
}

int GetDimensionLength(const TensorShape& shape, int dimension) {
  return shape.dimension()->Get(dimension)->end() -
         shape.dimension()->Get(dimension)->start() + 1;
}

int GetDimensionLength(const TensorShapeT& shape, int dimension) {
  return shape.dimension.at(dimension).end() -
         shape.dimension.at(dimension).start() + 1;
}

bool IsElementInShape(const TensorShape& shape,
                      const std::vector<int>& position) {
  CHECK_EQ(position.size(), shape.dimension()->size());
  for (int i = 0; i < shape.dimension()->size(); ++i) {
    const auto range = shape.dimension()->Get(i);
    if (position[i] < range->start() || position[i] > range->end()) {
      return false;
    }
  }
  return true;
}

bool IsElementInShape(const TensorShapeT& shape,
                      const std::vector<int>& position) {
  CHECK_EQ(position.size(), shape.dimension.size());
  for (int i = 0; i < shape.dimension.size(); ++i) {
    const auto& range = shape.dimension[i];
    if (position[i] < range.start() || position[i] > range.end()) {
      return false;
    }
  }
  return true;
}

std::unique_ptr<TensorLayoutT> BuildPackedLayout(const TensorShape& shape) {
  auto layout = gtl::MakeUnique<TensorLayoutT>();

  // Fill shape information.
  layout->shape = gtl::MakeUnique<TensorShapeT>();
  shape.UnPackTo(layout->shape.get());

  // Fill stride information.
  layout->stride.resize(layout->shape->dimension.size());
  int current_stride = 1;
  for (int i = layout->shape->dimension.size() - 1; i >= 0; --i) {
    layout->stride[i] = current_stride;
    current_stride *= GetDimensionLength(*layout->shape, i);
  }

  return layout;
}

std::unique_ptr<TensorLayoutT> BuildPackedLayout(const TensorShapeT& shape) {
  auto layout = gtl::MakeUnique<TensorLayoutT>();

  // Fill shape information.
  layout->shape = gtl::MakeUnique<TensorShapeT>(shape);

  // Fill stride information.
  layout->stride.resize(layout->shape->dimension.size());
  int current_stride = 1;
  for (int i = layout->shape->dimension.size() - 1; i >= 0; --i) {
    layout->stride[i] = current_stride;
    current_stride *= GetDimensionLength(*layout->shape, i);
  }

  return layout;
}

bool IsValidLayout(const TensorLayout& layout) {
  const auto& shape = *layout.shape();
  if (!IsValidShape(shape)) {
    return false;
  }

  for (int i = 0; i < shape.dimension()->size() - 1; ++i) {
    if (layout.stride()->Get(i) <
        layout.stride()->Get(i + 1) * GetDimensionLength(shape, i + 1)) {
      return false;
    }
  }

  return true;
}

bool IsValidLayout(const TensorLayoutT& layout) {
  const auto& shape = *layout.shape;
  if (!IsValidShape(shape)) {
    return false;
  }

  for (int i = 0; i < shape.dimension.size() - 1; ++i) {
    if (layout.stride[i] <
        layout.stride[i + 1] * GetDimensionLength(shape, i + 1)) {
      return false;
    }
  }

  return true;
}

bool IsNoPaddingLayout(const TensorLayout& layout) {
  CHECK(IsValidLayout(layout));
  const auto& shape = *layout.shape();

  // There's no padding in layout if the stride equals to the dimension size.
  for (int i = 0; i < shape.dimension()->size() - 1; ++i) {
    if (layout.stride()->Get(i) !=
        layout.stride()->Get(i + 1) * GetDimensionLength(shape, i + 1)) {
      return false;
    }
  }
  return true;
}

bool IsNoPaddingLayout(const TensorLayoutT& layout) {
  DCHECK(IsValidLayout(layout));
  const auto& shape = *layout.shape;

  // There's no padding in layout if the stride equals to the dimension size.
  for (int i = 0; i < shape.dimension.size() - 1; ++i) {
    if (layout.stride[i] !=
        layout.stride[i + 1] * GetDimensionLength(shape, i + 1)) {
      return false;
    }
  }
  return true;
}

int GetLayoutSizeInElements(const TensorLayout& layout) {
  CHECK(IsValidLayout(layout));
  return GetDimensionLength(*layout.shape(), 0) * layout.stride()->Get(0);
}

int GetLayoutSizeInElements(const TensorLayoutT& layout) {
  CHECK(IsValidLayout(layout)) << DumpLayout(layout);
  return GetDimensionLength(*layout.shape, 0) * layout.stride[0];
}

int GetMemoryIndexFromPosition(const TensorLayout& layout,
                               const std::vector<int>& position) {
  CHECK(IsElementInShape(*layout.shape(), position));
  int memory_index = 0;
  for (int i = 0; i < position.size(); ++i) {
    const int min_index = layout.shape()->dimension()->Get(i)->start();
    const int stride = layout.stride()->Get(i);
    memory_index += stride * (position[i] - min_index);
  }
  return memory_index;
}

int GetMemoryIndexFromPosition(const TensorLayoutT& layout,
                               const std::vector<int>& position) {
  CHECK(IsElementInShape(*layout.shape, position));
  int memory_index = 0;
  for (int i = 0; i < position.size(); ++i) {
    const int min_index = layout.shape->dimension.at(i).start();
    const int stride = layout.stride.at(i);
    memory_index += stride * (position[i] - min_index);
  }
  return memory_index;
}

int GetFirstMemoryIndexForShape(const TensorLayout& layout,
                                const TensorShapeT& shape) {
  std::vector<int> position(shape.dimension.size());
  for (int i = 0; i < shape.dimension.size(); ++i) {
    position[i] = shape.dimension[i].start();
  }
  return GetMemoryIndexFromPosition(layout, position);
}

int GetLastMemoryIndexForShape(const TensorLayout& layout,
                               const TensorShapeT& shape) {
  std::vector<int> position(shape.dimension.size());
  for (int i = 0; i < shape.dimension.size(); ++i) {
    position[i] = shape.dimension[i].end();
  }
  return GetMemoryIndexFromPosition(layout, position);
}

bool IsShapeInContiguousLayout(const TensorLayout& layout,
                               const TensorShapeT& shape) {
  const int first_index = GetFirstMemoryIndexForShape(layout, shape);
  const int last_index = GetLastMemoryIndexForShape(layout, shape);
  return GetNumElementsInShape(shape) == (last_index - first_index + 1);
}

std::string DumpShape(const TensorShape& shape) {
  std::string str;
  for (int i = 0; i < shape.dimension()->size(); ++i) {
    const auto range = shape.dimension()->Get(i);
    StrAppend(&str, StringPrintf("[%d:%d]", range->start(), range->end()));
  }
  return str;
}

std::string DumpShape(const TensorShapeT& shape) {
  std::string str;
  for (int i = 0; i < shape.dimension.size(); ++i) {
    StrAppend(&str, StringPrintf("[%d:%d]", shape.dimension[i].start(),
                                 shape.dimension[i].end()));
  }
  return str;
}

std::string DumpLayout(const TensorLayout& layout) {
  std::string str =
      StringPrintf("shape=%s", DumpShape(*layout.shape()).c_str());
  StrAppend(&str, ",stride=");
  for (int i = 0; i < layout.stride()->size(); ++i) {
    if (i > 0) {
      StrAppend(&str, "/");
    }
    StrAppend(&str, StringPrintf("%d", layout.stride()->Get(i)));
  }
  return str;
}

std::string DumpLayout(const TensorLayoutT& layout) {
  std::string str = StringPrintf("shape=%s", DumpShape(*layout.shape).c_str());
  StrAppend(&str, ",stride=");
  for (int i = 0; i < layout.stride.size(); ++i) {
    if (i > 0) {
      StrAppend(&str, "/");
    }
    StrAppend(&str, StringPrintf("%d", layout.stride[i]));
  }
  return str;
}

TensorShapeT GetMinimumBoundingShape(
    const std::vector<const TensorShape*>& shapes) {
  TensorShapeT merged_shape;
  auto& merged_shape_dims = merged_shape.dimension;
  int dimension_size = shapes[0]->dimension()->size();
  merged_shape_dims.resize(dimension_size, {INT_MAX, INT_MIN});
  for (int i = 0; i < shapes.size(); ++i) {
    const TensorShape& slice_shape = *shapes[i];
    for (int d = 0; d < dimension_size; ++d) {
      const auto* cur_dim = slice_shape.dimension()->Get(d);
      auto& dim = merged_shape_dims[d];
      dim.mutate_start(std::min(dim.start(), cur_dim->start()));
      dim.mutate_end(std::max(dim.end(), cur_dim->end()));
    }
  }
  return merged_shape;
}

}  // namespace tensor_util
}  // namespace api
}  // namespace darwinn
}  // namespace platforms
