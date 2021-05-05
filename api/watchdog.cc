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

#include "api/watchdog.h"

#include <chrono>  // NOLINT(build/c++11)
#include <climits>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/ptr_util.h"
#include "port/status.h"
#include "port/status_macros.h"
#include "port/std_mutex_lock.h"
#include "port/stringprintf.h"
#include "port/time.h"
#include "port/timer.h"

namespace platforms {
namespace darwinn {
namespace api {

namespace {

inline int64 GetNextActivationId(int64 current_id) {
  return (current_id == INT64_MAX) ? 0 : current_id + 1;
}

}  // namespace

std::unique_ptr<Watchdog> Watchdog::MakeWatchdog(int64 timeout_ns,
                                                 Expire expire) {
  if (timeout_ns > 0) {
    return gtl::MakeUnique<TimerFdWatchdog>(timeout_ns, expire);
  }
  return gtl::MakeUnique<NoopWatchdog>();
}

TimerFdWatchdog::TimerFdWatchdog(int64 timeout_ns, Expire expire)
    : TimerFdWatchdog(timeout_ns, std::move(expire),
                      gtl::MakeUnique<Timer>()) {}

TimerFdWatchdog::TimerFdWatchdog(int64 timeout_ns, Expire expire,
                                 std::unique_ptr<Timer> timer)
    : expire_(std::move(expire)),
      timeout_ns_(timeout_ns),
      timer_(std::move(timer)) {
  CHECK_GT(timeout_ns_, 0);
  watcher_thread_ = std::thread([this]() { Watcher(); });
}

TimerFdWatchdog::~TimerFdWatchdog() {
  {
    StdMutexLock lock(&mutex_);
    // 'DESTROYED' indicates that the watcher_thread_ should exit the loop.
    // In case the watchdog is still BARKING, we set it to DESTROYED, so the
    // watcher_thread_ can gracefully exit after the callback returns. The only
    // side effect is that state_ is DESTROYED even though callback is running.
    // Should be okay since nobody will query the watchdog state after this.
    CHECK(state_ == WatchdogState::INACTIVE ||
          state_ == WatchdogState::BARKING);
    state_ = WatchdogState::DESTROYED;
    CHECK_OK(timer_->Set(1));
  }

  watcher_thread_.join();
}

StatusOr<int64> TimerFdWatchdog::Activate() {
  StdMutexLock lock(&mutex_);
  switch (state_) {
    case WatchdogState::ACTIVE:
      break;  // Already active: Return old activation_id.
    case WatchdogState::BARKING:
      VLOG(1) << "A barking watchdog was re-activated.";
      RETURN_IF_ERROR(timer_->Set(timeout_ns_));
      state_ = WatchdogState::ACTIVE;
      activation_id_ = GetNextActivationId(activation_id_);
      break;
    case WatchdogState::INACTIVE:
      VLOG(5) << "Activating the watchdog.";
      RETURN_IF_ERROR(timer_->Set(timeout_ns_));
      state_ = WatchdogState::ACTIVE;
      activation_id_ = GetNextActivationId(activation_id_);
      break;
    case WatchdogState::DESTROYED:
      return FailedPreconditionError("Cannot activate a destroyed watchdog.");
  }
  return activation_id_;
}

Status TimerFdWatchdog::Signal() {
  StdMutexLock lock(&mutex_);
  switch (state_) {
    case WatchdogState::ACTIVE:
      VLOG(5) << "Signalling the watchdog.";
      RETURN_IF_ERROR(timer_->Set(timeout_ns_));
      return OkStatus();
    case WatchdogState::BARKING:
      return OkStatus();
    case WatchdogState::INACTIVE:
    case WatchdogState::DESTROYED:
      return FailedPreconditionError(
          "Cannot signal an in-active / destroyed watchdog.");
  }
}

Status TimerFdWatchdog::Deactivate() {
  StdMutexLock lock(&mutex_);
  switch (state_) {
    case WatchdogState::ACTIVE:
      VLOG(5) << "De-activating an active watchdog.";
      RETURN_IF_ERROR(timer_->Set(0));
      state_ = WatchdogState::INACTIVE;
      return OkStatus();
    case WatchdogState::BARKING:
    case WatchdogState::INACTIVE:
      // Watchdog is either inactive or will become inactive. Nothing to do.
      return OkStatus();
    case WatchdogState::DESTROYED:
      return FailedPreconditionError("Cannot deactivate a destroyed watchdog.");
  }
}

Status TimerFdWatchdog::UpdateTimeout(int64 timeout_ns) {
  if (timeout_ns <= 0) {
    return InvalidArgumentError(StringPrintf(
        "Watchdog timeout should be a positive integer. %lld was provided",
        static_cast<long long>(timeout_ns)));
  }

  StdMutexLock lock(&mutex_);
  timeout_ns_ = timeout_ns;

  return OkStatus();
}

void TimerFdWatchdog::Watcher() {
  while (true) {
    auto expirations_or_error = timer_->Wait();
    CHECK_OK(expirations_or_error.status());
    auto expirations = expirations_or_error.ValueOrDie();
    if (expirations == 0) {
      continue;
    }
    CHECK_EQ(expirations, 1);

    // Local copies of shared state to be used when we don't have the lock.
    int64 activation_id = 0;
    bool do_expire = false;

    // Acquire lock to query and update shared state.
    {
      StdMutexLock lock(&mutex_);

      if (state_ == WatchdogState::DESTROYED) {
        VLOG(5) << "Callback watcher thread ended.";
        return;
      }

      if (state_ != WatchdogState::ACTIVE) {
        VLOG(1) << "Timer got triggered but watchdog is not active.";
        continue;
      }

      do_expire = true;
      state_ = WatchdogState::BARKING;
      activation_id = activation_id_;
    }

    if (do_expire) {
      // Callback occurs outside locked region since it might take more time.
      VLOG(2) << "Calling watchdog expiration callback with ID:"
              << activation_id;
      expire_(activation_id);

      // Acquire lock again to update shared state after calling expire_.
      {
        StdMutexLock lock(&mutex_);
        // While the watchdog was executing the expire_ callback (ie BARKING):
        //  If ~Watchdog was called, retain DESTROYED state.
        //  If Activate was called (re-activated), retain ACTIVE state.
        //  If Deactivate was called, state will change to INACTIVE now.
        if (state_ == WatchdogState::BARKING) {
          state_ = WatchdogState::INACTIVE;
        }
      }
    }
  }
}

CountingWatch::~CountingWatch() {
  StdMutexLock lock(&mutex_);
  if (counter_ != 0) {
    LOG(WARNING) << StringPrintf(
        "Destructing counting watch while counter is %lld",
        static_cast<long long>(counter_));
  }
}

Status CountingWatch::Increment() {
  StdMutexLock lock(&mutex_);

  if (counter_ == LLONG_MAX) {
    return InternalError("Reached max counter value.");
  }

  counter_++;
  VLOG(5) << StringPrintf("Incrementing watch counter to %lld.",
                          static_cast<long long>(counter_));
  return watchdog_->Activate().status();
}

Status CountingWatch::Decrement() {
  StdMutexLock lock(&mutex_);
  if (counter_ <= 0) {
    return FailedPreconditionError(
        StringPrintf("Cannot decrement when counter is %lld.",
                     static_cast<long long>(counter_)));
  }

  counter_--;
  VLOG(5) << StringPrintf("Decrementing watch counter to %lld.",
                          static_cast<long long>(counter_));

  RETURN_IF_ERROR(watchdog_->Signal());

  if (counter_ == 0) {
    RETURN_IF_ERROR(watchdog_->Deactivate());
  }

  return OkStatus();
}

CascadeWatchdog::CascadeWatchdog(const std::vector<Config>& configs)
    : CascadeWatchdog(configs, [](int64 timeout_ns, Expire expire) {
        return gtl::MakeUnique<TimerFdWatchdog>(timeout_ns, std::move(expire));
      }) {}

CascadeWatchdog::CascadeWatchdog(const std::vector<Config>& configs,
                                 WatchdogMaker make_watchdog)
    : configs_(configs) {
  CHECK_GT(configs.size(), 0);
  watchdogs_.reserve(configs.size());

  expiration_callback_thread_ = std::thread([this]() { CallbackExecutor(); });

  // Set callbacks for each watchdog. Note that there are 3 levels of callback.
  // 'make_watchdog' has an anonymous method that calls 'WatchdogExpired', which
  // in turn does some checks / book-keeping and invokes the actual callback
  // that is registered in the 'configs_' vector.
  for (int i = 0; i < configs.size(); ++i) {
    watchdogs_.push_back(make_watchdog(
        configs[i].timeout_ns,
        [this, i](int64 activation_id) { WatchdogExpired(activation_id, i); }));
  }
}

CascadeWatchdog::~CascadeWatchdog() {
  {
    StdMutexLock lock(&mutex_);
    is_alive_ = false;
    child_expired_.notify_one();
  }

  expiration_callback_thread_.join();
}

void CascadeWatchdog::WatchdogExpired(int64 child_activation_id, int child_id) {
  StdMutexLock lock(&mutex_);
  if (child_activation_id != child_activation_id_ ||
      child_id != currently_active_) {
    // This means this is a delayed callback for an earlier activation, we
    // should skip it.
    return;
  }

  auto expire = configs_[currently_active_].expire;
  auto activation_id = activation_id_;
  expirations_.push_back([expire, activation_id]() { expire(activation_id); });
  child_expired_.notify_one();

  if (currently_active_ < watchdogs_.size() - 1) {
    ++currently_active_;
    child_activation_id_ =
        watchdogs_[currently_active_]->Activate().ValueOrDie();
  } else {
    currently_active_ = kNoneActive;
  }
}

Status CascadeWatchdog::StartFirstWatchdog() {
  ASSIGN_OR_RETURN(child_activation_id_, watchdogs_[0]->Activate());
  currently_active_ = 0;
  return OkStatus();
}

Status CascadeWatchdog::DeactivateInternal() {
  if (currently_active_ == kNoneActive) {
    return OkStatus();
  }

  // There is a chance that we end up deactivating an already expired watchdog
  // which will result in this call returning OK status but still getting the
  // callback. However, callback notices that currently_active_ = kNoneActive
  // and does not execute the expiration function.
  RETURN_IF_ERROR(watchdogs_[currently_active_]->Deactivate());
  currently_active_ = kNoneActive;

  return OkStatus();
}

StatusOr<int64> CascadeWatchdog::Activate() {
  StdMutexLock lock(&mutex_);
  if (currently_active_ != kNoneActive) {
    return activation_id_;
  }
  RETURN_IF_ERROR(StartFirstWatchdog());
  activation_id_ = GetNextActivationId(activation_id_);
  return activation_id_;
}

Status CascadeWatchdog::Signal() {
  // Early exit if watchdog is not active
  StdMutexLock lock(&mutex_);
  if (currently_active_ == kNoneActive) {
    VLOG(2) << "Signalled inactive CascadeWatchdog. Ignoring.";
    return OkStatus();
  }

  RETURN_IF_ERROR(DeactivateInternal());
  return StartFirstWatchdog();
}

Status CascadeWatchdog::Deactivate() {
  StdMutexLock lock(&mutex_);
  return DeactivateInternal();
}

Status CascadeWatchdog::UpdateTimeout(int64 timeout_ns) {
  return watchdogs_[0]->UpdateTimeout(timeout_ns);
}

Status CascadeWatchdog::UpdateTimeout(int child_index, int64 timeout_ns) {
  if (child_index >= watchdogs_.size()) {
    return InvalidArgumentError(StringPrintf(
        "Invalid child_index %d. We only have %zu child watchdogs.",
        child_index, watchdogs_.size()));
  }
  return watchdogs_[child_index]->UpdateTimeout(timeout_ns);
}

void CascadeWatchdog::CallbackExecutor() {
  while (true) {
    std::vector<std::function<void()>> expirations;

    {
      StdCondMutexLock lock(&mutex_);
      while (expirations_.empty() && is_alive_) {
        child_expired_.wait(lock);
      }

      if (!is_alive_) {
        return;
      }

      expirations = std::move(expirations_);
      expirations_.clear();
    }

    for (const auto& expiration : expirations) {
      expiration();
    }
  }
}

}  // namespace api
}  // namespace darwinn
}  // namespace platforms
