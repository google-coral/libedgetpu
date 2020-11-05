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

#include "api/layer_information.h"

#include <string>
#include <vector>

#include "api/buffer.h"
#include "api/tensor_util.h"
#include "executable/executable_generated.h"
#include "port/errors.h"
#include "port/logging.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace api {

namespace {

// Performs sanity check for the output shape information. Returns error if
// slice layout information is invalid.
util::Status SanityCheckShapeInformation(const OutputShapeInfo& shape_info,
                                         int data_type_size) {
  for (int i = 0; i < shape_info.slice_layout()->size(); ++i) {
    // Each slice shape is stored in its own slice layout. Make sure the layout
    // is valid.
    const auto& slice_layout = *shape_info.slice_layout()->Get(i);
    if (!tensor_util::IsValidLayout(slice_layout)) {
      return util::FailedPreconditionError(
          StringPrintf("Invalid shape for slice %d: %s", i,
                       tensor_util::DumpLayout(slice_layout).c_str()));
    }
    const int slice_offset = shape_info.slice_offset()->Get(i);
    if (slice_offset % data_type_size != 0) {
      return util::FailedPreconditionError(StringPrintf(
          "Slice offset [%d] is not aliged to data type size [%d].",
          slice_offset, data_type_size));
    }
  }

  return util::OkStatus();
}

// Copies elements in source shape to dest address. Destination layout is in
// dest_shape.
void CopyShape(const TensorShapeT& source_shape,
               const TensorLayout& source_layout,
               const unsigned char* source_address,
               const TensorLayout& dest_layout, unsigned char* dest_address,
               int bytes_per_element, int dimension) {
  CHECK_LT(dimension, tensor_util::kNumDimensions);
  CHECK_GE(dimension, 0);

  // Source shape can be in non-contiguous memory space if there are z-padding
  // elements.
  if (tensor_util::IsShapeInContiguousLayout(source_layout, source_shape) &&
      tensor_util::IsShapeInContiguousLayout(dest_layout, source_shape)) {
    const int dest_offset =
        tensor_util::GetFirstMemoryIndexForShape(dest_layout, source_shape) *
        bytes_per_element;
    const int source_offset =
        tensor_util::GetFirstMemoryIndexForShape(source_layout, source_shape) *
        bytes_per_element;

    // Do mem copy for the shape.
    memcpy(
        dest_address + dest_offset, source_address + source_offset,
        tensor_util::GetNumElementsInShape(source_shape) * bytes_per_element);
  } else {
    const auto range = source_shape.dimension.at(dimension);
    for (int i = range.start(); i <= range.end(); ++i) {
      auto slice = source_shape;
      slice.dimension.at(dimension) = {i, i};
      CopyShape(slice, source_layout, source_address, dest_layout, dest_address,
                bytes_per_element, dimension + 1);
    }
  }
}

}  // namespace

LayerInformation::LayerInformation(const Layer* layer) : layer_(layer) {
  CHECK(layer != nullptr);
}

int LayerInformation::DataTypeSize() const {
  return TensorDataTypeSize(layer()->data_type());
}

bool LayerInformation::SignedDataType() const {
  switch (layer()->data_type()) {
    case DataType_SIGNED_FIXED_POINT8:
    case DataType_SIGNED_FIXED_POINT16:
      return true;

    case DataType_FIXED_POINT8:
    case DataType_FIXED_POINT16:
    // TODO: DataType_SIGNED_FIXED_POINT32 (previously
    // DataType_FIXED_POINT32) is a signed number, see b/135944737.
    // However, the function returns false, which looks like a bug. Please
    // confirm it.
    case DataType_SIGNED_FIXED_POINT32:
    case DataType_BFLOAT:
    case DataType_HALF:
    case DataType_SINGLE:
      return false;
  }
}

util::Status LayerInformation::TransformSignedDataType(Buffer buffer) const {
  const auto data_type_size = DataTypeSize();
  if (buffer.size_bytes() < ActualSizeBytes()) {
    return util::InvalidArgumentError(StringPrintf(
        "Provided buffer size (%zu) is less than actual size_bytes (%d).",
        buffer.size_bytes(), ActualSizeBytes()));
  }
  auto buffer_pointer = buffer.ptr();
  int buffer_index = 0;

  for (int y = 0; y < y_dim(); ++y) {
    for (int x = 0; x < x_dim(); ++x) {
      for (int z = 0; z < z_dim(); ++z) {
        // XORing with 128 on the last byte of each entry will flip the MSB of
        // each entry. Please note that bytes are stored little endian.
        int msb_index = buffer_index + data_type_size - 1;
        buffer_pointer[msb_index] = buffer_pointer[msb_index] ^ 128;
        buffer_index += data_type_size;
      }
    }
  }

  return util::OkStatus();
}

InputLayerInformation::InputLayerInformation(const Layer* layer)
    : LayerInformation(layer) {}

OutputLayerInformation::OutputLayerInformation(const Layer* layer)
    : LayerInformation(layer),
      output_layer_(layer->any_layer_as_OutputLayer()) {
  CHECK(output_layer_ != nullptr);
}

OutputLayerInformation::YBufferIndex OutputLayerInformation::GetYBufferIndex(
    int y) const {
  const auto& layout = output_layer_->layout();
  YBufferIndex output;
  output.y_linearized_tile_id =
      layout->y_coordinate_to_linear_tile_id_map()->Get(y);
  output.local_y_coordinate = layout->y_coordinate_to_local_y_offset()->Get(y);
  return output;
}

int OutputLayerInformation::GetBufferIndex(const YBufferIndex& y_buffer_index,
                                           int x, int z) const {
  const auto& layout = output_layer_->layout();
  const int linear_tile_id =
      y_buffer_index.y_linearized_tile_id +
      layout->x_coordinate_to_linear_tile_id_map()->Get(x);
  const int global_tile_byte_offset =
      layout->linearized_tile_byte_offset()->Get(linear_tile_id);

  const int local_x_byte_offset =
      layout->x_coordinate_to_local_byte_offset()->Get(x);
  const int local_y_byte_offset =
      y_buffer_index.local_y_coordinate *
      layout->x_coordinate_to_local_y_row_size()->Get(x);

  return global_tile_byte_offset + local_y_byte_offset + local_x_byte_offset +
         z;
}

int OutputLayerInformation::GetBufferIndex(int y, int x, int z) const {
  return GetBufferIndex(GetYBufferIndex(y), x, z);
}

bool OutputLayerInformation::NeedsRelayout() const {
  if (!output_layer_->shape_info()) {
    return true;
  }
  // Relayout is not needed when output layout has only one shape with no
  // padding between elements.
  const auto& slice_layouts = *output_layer_->shape_info()->slice_layout();
  return slice_layouts.size() > 1 ||
         !tensor_util::IsNoPaddingLayout(*slice_layouts.Get(0));
}

// TODO Add unit tests for this method.
util::Status OutputLayerInformation::Relayout(unsigned char* dest,
                                              const unsigned char* src) const {
  const auto data_type_size = DataTypeSize();
  const int z_bytes = z_dim() * data_type_size;
  const int executions = execution_count_per_inference();

  if (executions == 1) {
    // Handle case when execution count is equal to 1, since if execution count
    // is greater than 1, there might be padding data in-between.

    // TODO: re-use the same buffer and avoid an unnecessary
    // memcopy when relayout is not needed.
    if (!NeedsRelayout()) {
      memcpy(dest, src, batch_dim() * y_dim() * x_dim() * z_bytes);
      return util::OkStatus();
    }

    if (output_layer_->shape_info()) {
      // If output shape info exists in the executable, use the new re-layout
      // function. Currently, this is only enabled for models with multiple
      // batches.
      return RelayoutWithShapeInformation(dest, src);
    }
  } else if (PaddedSizeBytes() == ActualSizeBytes() && !NeedsRelayout()) {
    // Use memcpy if `executions` is greater than 1 and there is no internal
    // padding between iterations.
    if (dest != src) {
      memcpy(dest, src, ActualSizeBytes());
    }
    return util::OkStatus();
  }

  if (y_dim() == 1 && x_dim() == 1) {
    // One dimensional output (only z-dimension).
    if (src != dest) {
      const int padded_size_bytes = PaddedSizeBytes();
      const int actual_size_bytes = ActualSizeBytes();
      if (executions == 1 || padded_size_bytes == actual_size_bytes) {
        memcpy(dest, src, z_bytes * executions);
      } else {
        // Remove padding values at the end of each execution.
        const int padded_size_per_execution =
            (padded_size_bytes - actual_size_bytes) / executions;
        for (int i = 0; i < executions; ++i) {
          memcpy(dest, src, z_bytes);
          dest += z_bytes;
          src += z_bytes + padded_size_per_execution;
        }
      }
    }
  } else {
    int z_bytes_padded;
    if (x_dim() > 1) {
      // If x-dim is > 1, padded-z-size can be deduced by looking at difference
      // between offset of element y=0,x=0,z=0 and y=0,x=1,z=0.
      z_bytes_padded = GetBufferIndex(0, 1, 0) - GetBufferIndex(0, 0, 0);
    } else {
      // Otherwise when x-dim is 1 (y-dim must be > 1 in that case),
      // padded-z-size can be deduced by looking at difference between offset of
      // element y=0,x=0,z=0 and y=1,x=0,z=0.
      z_bytes_padded = GetBufferIndex(1, 0, 0) - GetBufferIndex(0, 0, 0);
    }
    z_bytes_padded *= data_type_size;

    const auto* layout = output_layer_->layout();
    int last_x = 0;
    int last_x_tile = layout->x_coordinate_to_linear_tile_id_map()->Get(0);
    std::vector<int> active_tile_x_sizes;
    for (int x = 1; x < x_dim(); ++x) {
      int cur_x_tile = layout->x_coordinate_to_linear_tile_id_map()->Get(x);
      if (cur_x_tile != last_x_tile) {
        active_tile_x_sizes.push_back(x - last_x);
        last_x_tile = cur_x_tile;
        last_x = x;
      }
    }
    active_tile_x_sizes.push_back(x_dim() - last_x);

// When the num_z_bytes parameter is a compile-time constant, the conditions
// in the innermost loop will be replaced with a single optimized path,
// specialized for that value.
// Specialization is provided for num_z_bytes value of 1 and 3.
// We can also make this a helper function and still realize the benefits
// provided we have a guaranteed way of ensuring this function would be inlined
// so that the compiler optimizations based on compile-time-constants can kick
// in.
#define RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(num_z_bytes, num_z_bytes_padded) \
  do {                                                                        \
    for (int y = 0; y < y_dim(); ++y) {                                       \
      const auto y_buffer_index = GetYBufferIndex(y);                         \
      int tile_starting_x = 0;                                                \
      for (int x_tile = 0; x_tile < active_tile_x_sizes.size(); ++x_tile) {   \
        const unsigned char* source =                                         \
            src + GetBufferIndex(y_buffer_index, tile_starting_x, /*z=*/0) *  \
                      data_type_size;                                         \
        const int tile_x_size = active_tile_x_sizes[x_tile];                  \
        for (int local_offset_x = 0; local_offset_x < tile_x_size;            \
             ++local_offset_x) {                                              \
          if ((num_z_bytes) == 1) {                                           \
            *dest = *source;                                                  \
          } else if ((num_z_bytes) == 3) {                                    \
            *(dest + 0) = *(source + 0);                                      \
            *(dest + 1) = *(source + 1);                                      \
            *(dest + 2) = *(source + 2);                                      \
          } else {                                                            \
            memcpy(dest, source, (num_z_bytes));                              \
          }                                                                   \
          dest += (num_z_bytes);                                              \
          source += (num_z_bytes_padded);                                     \
        }                                                                     \
        tile_starting_x += tile_x_size;                                       \
      }                                                                       \
    }                                                                         \
  } while (0)

    if (z_bytes != z_bytes_padded) {
      if (z_bytes == 1) {
        // Specialization for z_bytes = 1 (grayscale image).
        RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(1, 4);
      } else if (z_bytes == 3) {
        // Specialization for z_bytes = 3 (RGB image).
        RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(3, 4);
      } else {
        // Default.
        RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(z_bytes, z_bytes_padded);
      }
    } else {
      // TODO: test models impacted with this relayout method.
      const int first_y_tile =
          layout->y_coordinate_to_linear_tile_id_map()->Get(0);
      const int last_y_tile =
          layout->y_coordinate_to_linear_tile_id_map()->Get(y_dim() - 1);

      // If there's only one output shape from one tile and no Z padding, copy
      // data directly.
      const bool need_relayout =
          active_tile_x_sizes.size() > 1 || first_y_tile != last_y_tile;

      if (need_relayout) {
        // TODO: If iteration count is more than 1, we need to make
        // sure we advance 'src' and 'dest' correctly due to padding issue. We
        // don't have test case now.
        CHECK_EQ(executions, 1)
            << "Verification is missing if execution count is greater than 1";

        // If there's no z padding, copy one xz block on one tile at a time.
        for (int y = 0; y < y_dim(); ++y) {
          const auto y_buffer_index = GetYBufferIndex(y);
          int tile_starting_x = 0;
          for (int x_tile = 0; x_tile < active_tile_x_sizes.size(); ++x_tile) {
            const unsigned char* source =
                src + GetBufferIndex(y_buffer_index, tile_starting_x, /*z=*/0) *
                          data_type_size;
            const int tile_x_size = active_tile_x_sizes[x_tile];
            const int tile_x_z_bytes = z_bytes * tile_x_size;
            memcpy(dest, source, tile_x_z_bytes);
            dest += tile_x_z_bytes;
            tile_starting_x += tile_x_size;
          }
        }
      } else {
        // TODO: avoid copy and assign in caller directly.
        memcpy(dest, src, x_dim() * y_dim() * z_bytes * executions);
      }
    }

#undef RELAYOUT_WITH_Z_BYTES_SPECIALIZATION
  }

  return util::OkStatus();
}

util::Status OutputLayerInformation::RelayoutWithShapeInformation(
    unsigned char* dest, const unsigned char* src) const {
  CHECK_EQ(execution_count_per_inference(), 1)
      << "Multiple inference execution not supported in the new relayout "
         "(b/129301507).";

  const auto data_type_size = DataTypeSize();
  const auto& shape_info = *output_layer_->shape_info();

  RETURN_IF_ERROR(SanityCheckShapeInformation(shape_info, data_type_size));

  flatbuffers::FlatBufferBuilder builder;
  const auto fb_layout = tensor_util::BuildPackedLayout(*layer()->shape());
  builder.Finish(darwinn::TensorLayout::Pack(builder, fb_layout.get()));
  const TensorLayout& dest_layout =
      *flatbuffers::GetRoot<TensorLayout>(builder.GetBufferPointer());
  unsigned char* dest_address = dest;

  const auto& slice_layouts = *shape_info.slice_layout();
  for (int i = 0; i < slice_layouts.size(); ++i) {
    // Each slice is stored in a contiguous memory space.
    const TensorLayout& source_layout = *slice_layouts.Get(i);
    TensorShapeT source_shape;
    source_layout.shape()->UnPackTo(&source_shape);
    const unsigned char* source_address =
        src + shape_info.slice_offset()->Get(i);

    CopyShape(source_shape, source_layout, source_address, dest_layout,
              dest_address, data_type_size, tensor_util::kBatch);
  }

  return util::OkStatus();
}

int OutputLayerInformation::GetBufferIndex(
    const std::vector<int>& element_position) const {
  const auto* shape_info = output_layer_->shape_info();
  if (!shape_info) {
    CHECK_EQ(element_position.size(), 4);
    CHECK_EQ(element_position[tensor_util::kBatch], 0);
    return GetBufferIndex(/*y=*/element_position[tensor_util::kY],
                          /*x=*/element_position[tensor_util::kX],
                          /*z=*/element_position[tensor_util::kZ]);
  }

  const int data_type_size = DataTypeSize();
  const auto& slice_layouts = *shape_info->slice_layout();
  for (int i = 0; i < slice_layouts.size(); ++i) {
    const TensorLayout& slice_layout = *slice_layouts.Get(i);
    const TensorShape& slice_shape = *slice_layout.shape();
    if (tensor_util::IsElementInShape(slice_shape, element_position)) {
      const int index = tensor_util::GetMemoryIndexFromPosition(
          slice_layout, element_position);
      const int slice_base_offset_in_bytes = shape_info->slice_offset()->Get(i);
      CHECK_EQ(slice_base_offset_in_bytes % data_type_size, 0);
      const int slice_base_offset_in_elements =
          slice_base_offset_in_bytes / data_type_size;
      return slice_base_offset_in_elements + index;
    }
  }

  std::string position_string;
  for (int index : element_position) {
    position_string += StringPrintf("[%d]", index);
  }

  LOG(FATAL) << "Cannot find element in output: " << position_string;
  return 0;
}

int TensorDataTypeSize(DataType data_type) {
  switch (data_type) {
    case DataType_FIXED_POINT8:
    case DataType_SIGNED_FIXED_POINT8:
      return 1;
    case DataType_FIXED_POINT16:
    case DataType_SIGNED_FIXED_POINT16:
      return 2;
    case DataType_SIGNED_FIXED_POINT32:
      return 4;
    case DataType_BFLOAT:
      return 2;
    case DataType_HALF:
      return 2;
    case DataType_SINGLE:
      return 4;
  }
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
