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

#ifndef DARWINN_DRIVER_DMA_SCHEDULER_H_
#define DARWINN_DRIVER_DMA_SCHEDULER_H_

#include <memory>
#include <vector>

#include "api/driver.h"
#include "driver/dma_info.h"
#include "driver/tpu_request.h"
#include "port/status.h"

namespace platforms {
namespace darwinn {
namespace driver {

// Manages the processing order of DMAs from DarwiNN Request, and also keeps
// track of the requests. All implementation of DMA scheduler has to be
// thread-safe.
//
// Example usage:
//   DmaScheduler scheduler;
//   scheduler.Submit(request0);
//   scheduler.Submit(request1);
//   ...
//   const auto* dma = scheduler.GetNextDma();
//   // Handle DMA.
//   if DMA is completed:
//     scheduler.NotifyDmaCompletion(dma);
//   ...
//   // when Request is complete
//   scheduler.NotifyRequestCompletion();
class DmaScheduler {
 public:
  DmaScheduler() = default;

  // This class is neither copyable nor movable.
  DmaScheduler(const DmaScheduler&) = delete;
  DmaScheduler& operator=(const DmaScheduler&) = delete;

  virtual ~DmaScheduler() = default;

  // Opens/closes DMA scheduler.
  virtual Status Open() = 0;
  virtual Status Close(api::Driver::ClosingMode mode) = 0;

  // Submits a request for execution on DarwiNN.
  virtual Status Submit(std::shared_ptr<TpuRequest> request) = 0;

  // Returns next DMA type to be performed. Returns kLocalFence if there is no
  // next DMA.
  virtual StatusOr<DmaDescriptorType> PeekNextDma() const = 0;

  // Returns DMA to perform. If there is no DMA to perform, returns nullptr.
  // Target of pointers are internally maintained.
  // DmaScheduler::NotifyDmaCompletion is a contract that given pointer is no
  // longer used by external entity.
  virtual StatusOr<DmaInfo*> GetNextDma() = 0;

  // Notifies that DMA for given "dma_info" has completed. Returns an error if
  // given "dma_info" cannot be completed.
  virtual Status NotifyDmaCompletion(DmaInfo* dma_info) = 0;

  // Notifies when request has been completed, and performs any necessary
  // cleanups.
  virtual Status NotifyRequestCompletion() = 0;

  // Cancels all the pending requests that has not been submitted to DarwiNN
  // device yet.
  virtual Status CancelPendingRequests() = 0;

  // Waits until active requests are done.
  virtual Status WaitActiveRequests() = 0;

  // Returns true if there is no DMAs to schedule.
  virtual bool IsEmpty() const = 0;

  // Returns the upper bound on number of TPU cycles remaining to complete all
  // scheduled tasks.
  virtual int64 MaxRemainingCycles() const = 0;

  // Returns the oldest submitted request that's still active.
  virtual StatusOr<std::shared_ptr<TpuRequest>> GetOldestActiveRequest()
      const = 0;
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_DMA_SCHEDULER_H_
