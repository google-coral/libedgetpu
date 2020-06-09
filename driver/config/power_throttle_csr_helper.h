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

#ifndef DARWINN_DRIVER_CONFIG_POWER_THROTTLE_CSR_HELPER_H_
#define DARWINN_DRIVER_CONFIG_POWER_THROTTLE_CSR_HELPER_H_

#include "driver/bitfield.h"
#include "port/integral_types.h"
#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {
namespace registers {

// Implements field level access for the EneryTable CSR register.
class EnergyTable {
 public:
  EnergyTable() : EnergyTable(/*value=*/0ULL) {}
  EnergyTable(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_idle_power(uint64 value) { reg_.idle_power_ = value; }
  uint64 idle_power() const { return reg_.idle_power_(); }

  void set_ring_bus(uint64 value) { reg_.ring_bus_ = value; }
  uint64 ring_bus() const { return reg_.ring_bus_(); }

  void set_nlu_active(uint64 value) { reg_.nlu_active_ = value; }
  uint64 nlu_active() const { return reg_.nlu_active_(); }

  void set_wide_byte_access(uint64 value) { reg_.wide_byte_access_ = value; }
  uint64 wide_byte_access() const { return reg_.wide_byte_access_(); }

  void set_narrow_mem_word_access(uint64 value) {
    reg_.narrow_mem_word_access_ = value;
  }
  uint64 narrow_mem_word_access() const {
    return reg_.narrow_mem_word_access_();
  }

  void set_int_multiply(uint64 value) { reg_.int_multiply_ = value; }
  uint64 int_multiply() const { return reg_.int_multiply_(); }

  void set_int_adder(uint64 value) { reg_.int_adder_ = value; }
  uint64 int_adder() const { return reg_.int_adder_(); }

  void set_float_32_adder(uint64 value) { reg_.float_32_adder_ = value; }
  uint64 float_32_adder() const { return reg_.float_32_adder_(); }

  void set_input_bus_transfer(uint64 value) {
    reg_.input_bus_transfer_ = value;
  }
  uint64 input_bus_transfer() const { return reg_.input_bus_transfer_(); }

  void set_wide_data_transfer(uint64 value) {
    reg_.wide_data_transfer_ = value;
  }
  uint64 wide_data_transfer() const { return reg_.wide_data_transfer_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 2> idle_power_;
    platforms::darwinn::driver::Bitfield<2, 2> ring_bus_;
    platforms::darwinn::driver::Bitfield<4, 2> nlu_active_;
    platforms::darwinn::driver::Bitfield<6, 2> wide_byte_access_;
    platforms::darwinn::driver::Bitfield<8, 2> narrow_mem_word_access_;
    platforms::darwinn::driver::Bitfield<10, 2> int_multiply_;
    platforms::darwinn::driver::Bitfield<12, 2> int_adder_;
    platforms::darwinn::driver::Bitfield<14, 2> float_32_adder_;
    platforms::darwinn::driver::Bitfield<16, 2> input_bus_transfer_;
    platforms::darwinn::driver::Bitfield<18, 2> wide_data_transfer_;
  } reg_;
};

// Implements field level access for *SampleInterval CSR.
class SampleInterval {
 public:
  SampleInterval() : SampleInterval(/*value=*/0ULL) {}
  SampleInterval(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_value(uint64 value) { reg_.value_ = value; }
  uint64 value() const { return reg_.value_(); }

  void set_enable(uint64 value) { reg_.enable_ = value; }
  uint64 enable() const { return reg_.enable_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 7> value_;
    platforms::darwinn::driver::Bitfield<7, 1> enable_;
  } reg_;
};

// Implements field level access for tdpSampleInterval CSR.
class tdpSampleInterval {
 public:
  tdpSampleInterval() : tdpSampleInterval(/*value=*/0ULL) {}
  tdpSampleInterval(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_value(uint64 value) { reg_.value_ = value; }
  uint64 value() const { return reg_.value_(); }

  void set_enable(uint64 value) { reg_.enable_ = value; }
  uint64 enable() const { return reg_.enable_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 14> value_;
    platforms::darwinn::driver::Bitfield<14, 1> enable_;
  } reg_;
};

// Implements field level access for *RunningSumInterval CSR.
class RunningSumInterval {
 public:
  RunningSumInterval() : RunningSumInterval(/*value=*/0ULL) {}
  RunningSumInterval(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_value(uint64 value) { reg_.value_ = value; }
  uint64 value() const { return reg_.value_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 2> value_;
  } reg_;
};

// Implements field level access for didtThreshold CSR.
class didtThreshold {
 public:
  didtThreshold() : didtThreshold(/*value=*/0ULL) {}
  didtThreshold(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_value(uint64 value) { reg_.value_ = value; }
  uint64 value() const { return reg_.value_(); }

  void set_enable(uint64 value) { reg_.enable_ = value; }
  uint64 enable() const { return reg_.enable_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 11> value_;
    platforms::darwinn::driver::Bitfield<11, 1> enable_;
  } reg_;
};

// Implements field level access for *ActionTable CSR.
class ActionTable {
 public:
  ActionTable() : ActionTable(/*value=*/0ULL) {}
  ActionTable(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_action0(uint64 value) { reg_.action0_ = value; }
  uint64 action0() const { return reg_.action0_(); }

  void set_action1(uint64 value) { reg_.action1_ = value; }
  uint64 action1() const { return reg_.action1_(); }

  void set_action2(uint64 value) { reg_.action2_ = value; }
  uint64 action2() const { return reg_.action2_(); }

  void set_action3(uint64 value) { reg_.action3_ = value; }
  uint64 action3() const { return reg_.action3_(); }

  void set_enable(uint64 value) { reg_.enable_ = value; }
  uint64 enable() const { return reg_.enable_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 3> action0_;
    platforms::darwinn::driver::Bitfield<3, 3> action1_;
    platforms::darwinn::driver::Bitfield<6, 3> action2_;
    platforms::darwinn::driver::Bitfield<9, 3> action3_;
    platforms::darwinn::driver::Bitfield<12, 1> enable_;
  } reg_;
};

}  // namespace registers
}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_POWER_THROTTLE_CSR_HELPER_H_
