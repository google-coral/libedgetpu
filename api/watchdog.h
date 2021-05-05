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

#ifndef DARWINN_API_WATCHDOG_H_
#define DARWINN_API_WATCHDOG_H_

#include <condition_variable>  // NOLINT(build/c++11)
#include <functional>
#include <mutex>   // NOLINT(build/c++11)
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "port/integral_types.h"
#include "port/status.h"
#include "port/statusor.h"
#include "port/thread_annotations.h"
#include "port/time.h"
#include "port/timer.h"

namespace platforms {
namespace darwinn {
namespace api {

// Watchdog is a class responsible for keeping track of TPU status and sending
// notifications when it is unresponsive.
class Watchdog {
 public:
  // A callback function to be called when the watch timeout is reached.
  using Expire = std::function<void(int64)>;

  Watchdog() = default;
  virtual ~Watchdog() = default;

  // This class is movable.
  Watchdog(Watchdog&& rhs);
  Watchdog& operator=(Watchdog&& rhs);

  // This class is not copyable.
  Watchdog(const Watchdog&) = delete;
  Watchdog& operator=(const Watchdog&) = delete;

  // Decides which watchdog concrete implementation to create based on the
  // provided parameters, creates and returns it.
  static std::unique_ptr<Watchdog> MakeWatchdog(int64 timeout_ns,
                                                Expire expire);

  // Starts the watch. It returns an activation id that can be later on
  // used to verify which activation an expiration callback belongs to.
  virtual StatusOr<int64> Activate() = 0;

  // Signals the watchdog that we are still active and healthy.
  virtual Status Signal() = 0;

  // Ends the watch.
  virtual Status Deactivate() = 0;

  // Updates watchdog timeout to the provided value in nanoseconds. By
  // definition, the new timeout will be effective from the next activation /
  // signal.
  virtual Status UpdateTimeout(int64 timeout_ns) = 0;
};

// A No-Op watchdog used for when we don't need a watch (e.g. in tests,
// simulator, etc.).
class NoopWatchdog : public Watchdog {
 public:
  NoopWatchdog() = default;
  ~NoopWatchdog() override = default;

  // This class is movable.
  NoopWatchdog(NoopWatchdog&& rhs);
  NoopWatchdog& operator=(NoopWatchdog&& rhs);

  // This class is not copyable.
  NoopWatchdog(const NoopWatchdog&) = delete;
  NoopWatchdog& operator=(const NoopWatchdog&) = delete;

  StatusOr<int64> Activate() override { return 0; }
  Status Signal() override { return OkStatus(); }
  Status Deactivate() override { return OkStatus(); }
  Status UpdateTimeout(int64 timeout_ns) override { return OkStatus(); }
};

// A watchdog implementation that uses timerfd (or similar timers) underneath.
class TimerFdWatchdog : public Watchdog {
 public:
  // This constructor uses timerfd system call.
  TimerFdWatchdog(int64 timeout_ns, Expire expire);

  // Accepts any timer interface. In most cases, it is recommended to use the
  // first constructor.
  TimerFdWatchdog(int64 timeout_ns, Expire expire,
                  std::unique_ptr<Timer> timer);

  ~TimerFdWatchdog() override;

  // This class is movable.
  TimerFdWatchdog(TimerFdWatchdog&& rhs);
  TimerFdWatchdog& operator=(TimerFdWatchdog&& rhs);

  // This class is not copyable.
  TimerFdWatchdog(const TimerFdWatchdog&) = delete;
  TimerFdWatchdog& operator=(const TimerFdWatchdog&) = delete;

  enum class WatchdogState {
    // State Transitions:
    //                         |```````````````````V
    // INACTIVE*-->ACTIVE-->BARKING-->INACTIVE-->DESTROYED
    //               ^--------------------^
    INACTIVE,  // Not yet activated or has finished barking.
    ACTIVE,    // Activated, but not yet barked - signal now to prevent barking.
    BARKING,   // Activated, and timer expired - callback is being executed.
    DESTROYED  // Watchdog Destructor has been called - exit watcher thread.
  };

  const char* GetStateString() const EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    switch (state_) {
      case WatchdogState::ACTIVE:
        return "ACTIVE";
      case WatchdogState::INACTIVE:
        return "INACTIVE";
      case WatchdogState::BARKING:
        return "BARKING";
      case WatchdogState::DESTROYED:
        return "DESTROYED";
    }
  }

  StatusOr<int64> Activate() override LOCKS_EXCLUDED(mutex_);
  Status Signal() override LOCKS_EXCLUDED(mutex_);
  Status Deactivate() override LOCKS_EXCLUDED(mutex_);
  Status UpdateTimeout(int64 timeout_ns) override LOCKS_EXCLUDED(mutex_);

 private:
  // This function runs the watch thread that periodically checks the last time
  // we heard anything.
  void Watcher();

  // Validates that the watchdog is currently active.
  Status ValidateActive() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Callback function for when we time out.
  const Expire expire_;

  // The amount of time watchdog has to be active and not get a signal in order
  // for it to expire.
  int64 timeout_ns_;

  // The timer to be used for keeping track of expiration deadlines.
  std::unique_ptr<Timer> timer_;

  // A single mutex to protect mutable fields in this class. This mutex is not
  // held while the watchdog callback is being executed. It is safe to signal,
  // activate, deactivate or destroy a barking watchdog.
  std::mutex mutex_;

  // Watchdog state machine:
  WatchdogState state_ GUARDED_BY(mutex_){WatchdogState::INACTIVE};

  // An id to verify the origin of an expiration callback.
  int64 activation_id_ GUARDED_BY(mutex_){0};

  // The watcher thread that runs Watcher() method.
  std::thread watcher_thread_;
};

// A wrapper around Watchdog that keeps track of device/code-state health by
// keeping track of the number of things in a pipeline.
class CountingWatch {
 public:
  // Constructor expects a configured watchdog. Its expiration callback is
  // called if Decrement is not called within the timeout and counter is not 0.
  explicit CountingWatch(std::unique_ptr<Watchdog> watchdog)
      : watchdog_(std::move(watchdog)) {}

  ~CountingWatch();

  // Increments the number of elements in the pipeline by 1. This will result in
  // activating the watchdog.
  Status Increment() LOCKS_EXCLUDED(mutex_);

  // Decrements the number of elements in the pipeline. It fails if counter has
  // already reached 0.
  Status Decrement() LOCKS_EXCLUDED(mutex_);

 private:
  // The watchdog we are wrapping.
  std::unique_ptr<Watchdog> watchdog_;

  // A mutex to ensure thread-safety for accessing members.
  std::mutex mutex_;

  // A counter to keep track of number of elements in the pipeline.
  int64 counter_ GUARDED_BY(mutex_) = 0;
};

// CascadeWatchdog is a multi-level watchdog that has an expiration callback and
// timeout for each level. After activation, if first level timeout expires,
// its callback function gets called and the second watch gets activated
// immediately after. Signaling or de-activating this watchdog resets everything
// back to first level.
class CascadeWatchdog : public Watchdog {
 public:
  // Encapsulates the configuration needed for each level in the cascade.
  struct Config {
    // Expiration function for when this watch level expires.
    Expire expire;

    // Timeout for triggering the watch (relative to the previous level).
    int64 timeout_ns;
  };

  // Creates a CascadeWatchdog provided a vector of configs. The configs are
  // used in the provided order meaning the first callback to get triggered is
  // the one in configs[0]. There has to be at least one config.
  explicit CascadeWatchdog(const std::vector<Config>& configs);

  ~CascadeWatchdog() override;

  // This class is movable.
  CascadeWatchdog(CascadeWatchdog&& rhs);
  CascadeWatchdog& operator=(CascadeWatchdog&& rhs);

  // This class is not copyable.
  CascadeWatchdog(const CascadeWatchdog&) = delete;
  CascadeWatchdog& operator=(const CascadeWatchdog&) = delete;

  StatusOr<int64> Activate() override LOCKS_EXCLUDED(mutex_);
  Status Signal() override LOCKS_EXCLUDED(mutex_);
  Status Deactivate() override LOCKS_EXCLUDED(mutex_);

  // Updates the timeout of the first child watchdog (the first one that expires
  // ). Use the overloaded method for updating timeouts of other child
  // watchdogs.
  Status UpdateTimeout(int64 timeout_ns) override;

  // Updates the timeout of the child watchdog at the provided index.
  Status UpdateTimeout(int child_index, int64 timeout_ns);

 protected:
  // A method that can create and return a child watchdog to be used here.
  using WatchdogMaker = std::function<std::unique_ptr<Watchdog>(int64, Expire)>;

  // A constructor that accepts a WatchdogMaker to use for creating the child
  // watchdogs.
  CascadeWatchdog(const std::vector<Config>& configs,
                  WatchdogMaker make_watchdog);

  // A vector of the underlying child watchdogs in the same order as configs.
  std::vector<std::unique_ptr<Watchdog>> watchdogs_;

 private:
  // The method that gets called in any of the child watchdogs expire.
  void WatchdogExpired(int64 child_activation_id, int child_id)
      LOCKS_EXCLUDED(mutex_);

  // The function responsible for executing expiration callbacks.
  void CallbackExecutor();

  // Start the first watchdog. Called by Activate and Signal.
  Status StartFirstWatchdog() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // Implement actual Deactivate method here to simplify some mutex locking.
  Status DeactivateInternal() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  // The list of watchdog configs as provided by the object owner.
  const std::vector<Config> configs_;

  // A mutex to protect mutable class fields.
  std::mutex mutex_;

  // Value that indicates all watchdogs are inactive.
  static constexpr int kNoneActive = -1;

  // Specifies which child watchdog is currently active. A value other than
  // kNoneActive can be used as an index to configs_ or watchdogs_.
  int currently_active_ GUARDED_BY(mutex_){kNoneActive};

  // The current/last generated activation ID for the caller of Activate on this
  // class.
  int64 activation_id_ GUARDED_BY(mutex_){0};

  // At any given point in time at most 1 child watchdog is active. This field
  // specifies the activation ID of that watchdog.
  int64 child_activation_id_ GUARDED_BY(mutex_){0};

  // A list of expiration callbacks that need to be executed.
  std::vector<std::function<void()>> expirations_ GUARDED_BY(mutex_);

  // To notify that at least one child watchdog has expired.
  std::condition_variable child_expired_;

  // The thread that executes expiration callbacks.
  std::thread expiration_callback_thread_;

  // Specifies if watchdog is alive. This is used in the destructor to signal
  // other threads that it is time to quit.
  bool is_alive_ GUARDED_BY(mutex_){true};
};

}  // namespace api
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_API_WATCHDOG_H_
