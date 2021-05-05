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

#ifndef DARWINN_API_LAYER_INFORMATION_H_
#define DARWINN_API_LAYER_INFORMATION_H_

#include <string>

#include "api/buffer.h"
#include "api/tensor_util.h"
#include "executable/executable_generated.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace api {

// Provides information on input and output layers.
class LayerInformation {
 public:
  virtual ~LayerInformation() = default;

  // Copyable.
  LayerInformation(const LayerInformation& rhs) = default;
  LayerInformation& operator=(const LayerInformation& rhs) = default;

  // Returns layer name.
  std::string name() const { return layer_->name()->str(); }

  // Layer dimensions.
  int x_dim() const { return layer_->x_dim(); }
  int y_dim() const { return layer_->y_dim(); }
  int z_dim() const { return layer_->z_dim(); }
  int batch_dim() const {
    if (layer_->shape()) {
      return tensor_util::GetDimensionLength(*layer_->shape(), 0);
    } else {
      return 1;
    }
  }

  // Returns the zero point value.
  int zero_point() const { return layer_->numerics()->zero_point(); }

  // Returns the execution count per inference.
  int execution_count_per_inference() const {
    return layer_->execution_count_per_inference();
  }

  // Returns the dequantization factor.
  float dequantization_factor() const {
    return layer_->numerics()->dequantization_factor();
  }

  // Returns data type in this layer.
  darwinn::DataType data_type() const { return layer_->data_type(); }

  // Returns the size of the data type in this layer in bytes.
  int DataTypeSize() const;

  // Returns true if the data type is signed.
  bool SignedDataType() const;

  // Returns the expected byte size of activations for given layer. This
  // excludes padding.
  int ActualSizeBytes() const {
    const int num_elements =
        (layer_->shape()) ? tensor_util::GetNumElementsInShape(*layer_->shape())
                          : x_dim() * y_dim() * z_dim();
    return num_elements * DataTypeSize() *
           layer_->execution_count_per_inference();
  }

  // Returns the expected size of activations for given layer including padding
  // bytes.
  int PaddedSizeBytes() const {
    return SizeBytesPerIteration() * layer_->execution_count_per_inference();
  }

  int SizeBytesPerIteration() const { return layer_->size_bytes(); }

  // Returns true if activations of this input/output layer need to be cached on
  // DRAM.
  bool CacheOnDram() const { return layer_->cache_on_dram(); }

  // Converts unsigned values for a provided buffer of this layer to signed and
  // vice versa.
  Status TransformSignedDataType(Buffer buffer) const;

 protected:
  explicit LayerInformation(const Layer* layer);

  const Layer* layer() const { return layer_; }

 private:
  const Layer* layer_;
};

// Provides detailed information on input layers.
class InputLayerInformation : public LayerInformation {
 public:
  explicit InputLayerInformation(const Layer* layer);
  ~InputLayerInformation() override = default;
};

// Provides detailed information on output layers.
class OutputLayerInformation : public LayerInformation {
 public:
  // Holds y-dependent values that are needed to calculate buffer index.
  // Expected usage is as follows:
  //
  // for (int y = 0; y < y_dim(); ++y) {
  //   const YBufferIndex y_buffer_index = GetYBufferIndex(y);
  //   for (int x = 0; x < x_dim(); ++x) {
  //     const int src_offset =
  //         GetBufferIndex(y_buffer_index, x, /*z=*/0) * data_type_size;
  //     ...
  //   }
  //  }
  struct YBufferIndex {
    // Holds the linearized tile ID for a given y value.
    int y_linearized_tile_id;
    // Holds local offset within a data chunk returned by a given tile.
    int local_y_coordinate;
  };

  explicit OutputLayerInformation(const Layer* layer);
  ~OutputLayerInformation() override = default;

  // Returns an index value of output buffer for a given tensor coordinate.
  int GetBufferIndex(int y, int x, int z) const;
  int GetBufferIndex(const std::vector<int>& element_position) const;

  // Essentially does the same thing as functions above, but these two split
  // functions allow a user to save some computation such as using the samve
  // value from GetYBufferIndex across all x pixels.
  YBufferIndex GetYBufferIndex(int y) const;
  int GetBufferIndex(const YBufferIndex& y_buffer_index, int x, int z) const;

  // Relayout the source DarwiNN output buffer (TYXZ layout, T = Tile) into
  // user output buffer (YXZ layout).
  //
  // TODO Move this method down to driver internal classes once all
  // dependencies are removed.
  Status Relayout(unsigned char* dest, const unsigned char* src) const;

  // Returns true if relayout is needed.
  bool NeedsRelayout() const;

  // Returns merged shape for output shapes.
  TensorShapeT GetMergedOutputShape() const;

 private:
  // Re-layouts the output activation stream from the tiles into a desired
  // format in the host memory.
  Status RelayoutWithShapeInformation(unsigned char* dest,
                                      const unsigned char* src) const;

  const OutputLayer* output_layer_;
};

// Returns the byte size of a provided tensor data type.
int TensorDataTypeSize(DataType data_type);

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_LAYER_INFORMATION_H_
