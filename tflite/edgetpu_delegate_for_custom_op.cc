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

#include "tflite/edgetpu_delegate_for_custom_op.h"

#include <cstring>
#include <vector>

#include "port/logging.h"
#include "tflite/public/edgetpu.h"
#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/context_util.h"
#include "tensorflow/lite/util.h"

using tflite::ConvertVectorToTfLiteIntArray;
using tflite::TfLiteIntArrayView;

namespace platforms {
namespace darwinn {
namespace tflite {
namespace {

constexpr char kDelegateName[] = "EdgeTpuDelegateForCustomOp";
constexpr int kDelegateVersion = 1;

void* DelegateInit(TfLiteContext* context, const char* buffer, size_t length) {
  const TfLiteDelegateParams* params =
      reinterpret_cast<const TfLiteDelegateParams*>(buffer);
  CHECK(params);

  TfLiteIntArray* nodes = params->nodes_to_replace;
  CHECK_EQ(nodes->size, 1);
  const int node_index = nodes->data[0];

  TfLiteNode* node;
  TfLiteRegistration* registration;
  CHECK(context->GetNodeAndRegistration(context, node_index, &node,
                                        &registration) == kTfLiteOk);

  return edgetpu::RegisterCustomOp()->init(
      context, static_cast<const char*>(node->custom_initial_data),
      node->custom_initial_data_size);
}

TfLiteStatus PrepareImpl(TfLiteContext* context, TfLiteDelegate* delegate) {
  context->SetExternalContext(
      context, kTfLiteEdgeTpuContext,
      static_cast<edgetpu::EdgeTpuContext*>(delegate->data_));

  TfLiteIntArray* plan;
  TF_LITE_ENSURE_STATUS(context->GetExecutionPlan(context, &plan));

  std::vector<int> edgetpu_nodes;
  for (int node_index : TfLiteIntArrayView(plan)) {
    TfLiteNode* node;
    TfLiteRegistration* registration;
    TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(
        context, node_index, &node, &registration));

    if (registration->custom_name &&
        std::strcmp(registration->custom_name, edgetpu::kCustomOp) == 0) {
      edgetpu_nodes.push_back(node_index);
    }
  }

  TfLiteRegistration registration = *edgetpu::RegisterCustomOp();
  registration.init = DelegateInit;
  registration.custom_name = kDelegateName;
  registration.version = kDelegateVersion;

  for (int node_index : edgetpu_nodes) {
    TfLiteIntArray* nodes = ConvertVectorToTfLiteIntArray({node_index});
    context->ReplaceNodeSubsetsWithDelegateKernels(
        context, registration, nodes, delegate);
    TfLiteIntArrayFree(nodes);
  }

  return kTfLiteOk;
}

class EdgeTpuDelegateForCustomOp : public TfLiteDelegate {
 public:
  EdgeTpuDelegateForCustomOp(std::shared_ptr<edgetpu::EdgeTpuContext> context)
      : TfLiteDelegate(TfLiteDelegateCreate()), context_(context) {
    this->data_ = context.get();
    this->Prepare = PrepareImpl;
    this->flags = kTfLiteDelegateFlagsAllowDynamicTensors;
  }

 private:
  std::shared_ptr<edgetpu::EdgeTpuContext> context_;
};

}  // namespace

TfLiteDelegate* CreateEdgeTpuDelegateForCustomOp(
    std::shared_ptr<edgetpu::EdgeTpuContext> context) {
  return context ? new EdgeTpuDelegateForCustomOp(context) : nullptr;
}

void FreeEdgeTpuDelegateForCustomOp(TfLiteDelegate* delegate) {
  delete static_cast<EdgeTpuDelegateForCustomOp*>(delegate);
}

}  // namespace tflite
}  // namespace darwinn
}  // namespace platforms
