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

#ifndef DARWINN_DRIVER_CONFIG_COMMON_CSR_HELPER_H_
#define DARWINN_DRIVER_CONFIG_COMMON_CSR_HELPER_H_

#include "driver/bitfield.h"
#include "port/integral_types.h"
#include "port/unreachable.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {
namespace registers {

// CSR helper to access fields for HibError* CSRs.
class HibError {
 public:
  HibError() : HibError(0ULL) {}
  explicit HibError(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_inbound_page_fault(uint64 value) {
    reg_.inbound_page_fault_ = value;
  }
  uint64 inbound_page_fault() const { return reg_.inbound_page_fault_(); }
  void set_extended_page_fault(uint64 value) {
    reg_.extended_page_fault_ = value;
  }
  uint64 extended_page_fault() const { return reg_.extended_page_fault_(); }
  void set_csr_parity_error(uint64 value) { reg_.csr_parity_error_ = value; }
  uint64 csr_parity_error() const { return reg_.csr_parity_error_(); }
  void set_axi_slave_b_error(uint64 value) { reg_.axi_slave_b_error_ = value; }
  uint64 axi_slave_b_error() const { return reg_.axi_slave_b_error_(); }
  void set_axi_slave_r_error(uint64 value) { reg_.axi_slave_r_error_ = value; }
  uint64 axi_slave_r_error() const { return reg_.axi_slave_r_error_(); }
  void set_instruction_queue_bad_configuration(uint64 value) {
    reg_.instruction_queue_bad_configuration_ = value;
  }
  uint64 instruction_queue_bad_configuration() const {
    return reg_.instruction_queue_bad_configuration_();
  }
  void set_input_actv_queue_bad_configuration(uint64 value) {
    reg_.input_actv_queue_bad_configuration_ = value;
  }
  uint64 input_actv_queue_bad_configuration() const {
    return reg_.input_actv_queue_bad_configuration_();
  }
  void set_param_queue_bad_configuration(uint64 value) {
    reg_.param_queue_bad_configuration_ = value;
  }
  uint64 param_queue_bad_configuration() const {
    return reg_.param_queue_bad_configuration_();
  }
  void set_output_actv_queue_bad_configuration(uint64 value) {
    reg_.output_actv_queue_bad_configuration_ = value;
  }
  uint64 output_actv_queue_bad_configuration() const {
    return reg_.output_actv_queue_bad_configuration_();
  }
  void set_instruction_queue_invalid(uint64 value) {
    reg_.instruction_queue_invalid_ = value;
  }
  uint64 instruction_queue_invalid() const {
    return reg_.instruction_queue_invalid_();
  }
  void set_input_actv_queue_invalid(uint64 value) {
    reg_.input_actv_queue_invalid_ = value;
  }
  uint64 input_actv_queue_invalid() const {
    return reg_.input_actv_queue_invalid_();
  }
  void set_param_queue_invalid(uint64 value) {
    reg_.param_queue_invalid_ = value;
  }
  uint64 param_queue_invalid() const { return reg_.param_queue_invalid_(); }
  void set_output_actv_queue_invalid(uint64 value) {
    reg_.output_actv_queue_invalid_ = value;
  }
  uint64 output_actv_queue_invalid() const {
    return reg_.output_actv_queue_invalid_();
  }
  void set_length_0_dma(uint64 value) { reg_.length_0_dma_ = value; }
  uint64 length_0_dma() const { return reg_.length_0_dma_(); }
  void set_virt_table_rdata_uncorr(uint64 value) {
    reg_.virt_table_rdata_uncorr_ = value;
  }
  uint64 virt_table_rdata_uncorr() const {
    return reg_.virt_table_rdata_uncorr_();
  }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 1> inbound_page_fault_;
    platforms::darwinn::driver::Bitfield<1, 1> extended_page_fault_;
    platforms::darwinn::driver::Bitfield<2, 1> csr_parity_error_;
    platforms::darwinn::driver::Bitfield<3, 1> axi_slave_b_error_;
    platforms::darwinn::driver::Bitfield<4, 1> axi_slave_r_error_;
    platforms::darwinn::driver::Bitfield<5, 1>
        instruction_queue_bad_configuration_;
    platforms::darwinn::driver::Bitfield<6, 1>
        input_actv_queue_bad_configuration_;
    platforms::darwinn::driver::Bitfield<7, 1> param_queue_bad_configuration_;
    platforms::darwinn::driver::Bitfield<8, 1>
        output_actv_queue_bad_configuration_;
    platforms::darwinn::driver::Bitfield<9, 1> instruction_queue_invalid_;
    platforms::darwinn::driver::Bitfield<10, 1> input_actv_queue_invalid_;
    platforms::darwinn::driver::Bitfield<11, 1> param_queue_invalid_;
    platforms::darwinn::driver::Bitfield<12, 1> output_actv_queue_invalid_;
    platforms::darwinn::driver::Bitfield<13, 1> length_0_dma_;
    platforms::darwinn::driver::Bitfield<14, 1> virt_table_rdata_uncorr_;
  } reg_;
};

// CSR helper to access fields for *QueueControl CSR.
class QueueControl {
 public:
  QueueControl() : QueueControl(0ULL) {}
  explicit QueueControl(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_enable(uint64 value) { reg_.enable_ = value; }
  uint64 enable() const { return reg_.enable_(); }
  void set_sc_desc_select(uint64 value) { reg_.sc_desc_select_ = value; }
  uint64 sc_desc_select() const { return reg_.sc_desc_select_(); }
  void set_sb_wr_enable(uint64 value) { reg_.sb_wr_enable_ = value; }
  uint64 sb_wr_enable() const { return reg_.sb_wr_enable_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 1> enable_;
    platforms::darwinn::driver::Bitfield<1, 1> sc_desc_select_;
    platforms::darwinn::driver::Bitfield<2, 1> sb_wr_enable_;
  } reg_;
};

// CSR helper to access fields for ScHostIntCount CSR.
class ScHostIntCount {
 public:
  ScHostIntCount() : ScHostIntCount(0ULL) {}
  explicit ScHostIntCount(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_cnt0(uint64 value) { reg_.cnt0_ = value; }
  uint64 cnt0() const { return reg_.cnt0_(); }
  void set_cnt1(uint64 value) { reg_.cnt1_ = value; }
  uint64 cnt1() const { return reg_.cnt1_(); }
  void set_cnt2(uint64 value) { reg_.cnt2_ = value; }
  uint64 cnt2() const { return reg_.cnt2_(); }
  void set_cnt3(uint64 value) { reg_.cnt3_ = value; }
  uint64 cnt3() const { return reg_.cnt3_(); }

  // Sets |index|-th field from LSB to |value|.
  void set_field(int index, uint64 value) {
    switch (index) {
      case 0:
        reg_.cnt0_ = value;
        break;

      case 1:
        reg_.cnt1_ = value;
        break;

      case 2:
        reg_.cnt2_ = value;
        break;

      case 3:
        reg_.cnt3_ = value;
        break;

      default:
        CHECK(false) << "Unknown field index: " << index;
        break;
    }
  }

  // Returns |index|-th field from LSB.
  uint64 get_field(int index) {
    switch (index) {
      case 0:
        return reg_.cnt0_();

      case 1:
        return reg_.cnt1_();

      case 2:
        return reg_.cnt2_();

      case 3:
        return reg_.cnt3_();

      default:
        LOG(FATAL) << "Unknown field index: " << index;
        unreachable();
    }
  }

  // Returns masked |value| for |index|-th field from LSB.
  uint64 mask_field(int index, uint64 value) {
    switch (index) {
      case 0:
        return value & reg_.cnt0_.mask();

      case 1:
        return value & reg_.cnt1_.mask();

      case 2:
        return value & reg_.cnt2_.mask();

      case 3:
        return value & reg_.cnt3_.mask();

      default:
        LOG(FATAL) << "Unknown field index: " << index;
        unreachable();
    }
  }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 16> cnt0_;
    platforms::darwinn::driver::Bitfield<16, 16> cnt1_;
    platforms::darwinn::driver::Bitfield<32, 16> cnt2_;
    platforms::darwinn::driver::Bitfield<48, 16> cnt3_;
  } reg_;
};

// CSR helper to access fields for ScHostIntStatus CSR.
class ScHostIntStatus {
 public:
  ScHostIntStatus() : ScHostIntStatus(0ULL) {}
  explicit ScHostIntStatus(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_hot0(uint64 value) { reg_.hot0_ = value; }
  uint64 hot0() const { return reg_.hot0_(); }
  void set_hot1(uint64 value) { reg_.hot1_ = value; }
  uint64 hot1() const { return reg_.hot1_(); }
  void set_hot2(uint64 value) { reg_.hot2_ = value; }
  uint64 hot2() const { return reg_.hot2_(); }
  void set_hot3(uint64 value) { reg_.hot3_ = value; }
  uint64 hot3() const { return reg_.hot3_(); }

  // Sets |index|-th field from LSB to |value|.
  void set_field(int index, uint64 value) {
    switch (index) {
      case 0:
        reg_.hot0_ = value;
        break;

      case 1:
        reg_.hot1_ = value;
        break;

      case 2:
        reg_.hot2_ = value;
        break;

      case 3:
        reg_.hot3_ = value;
        break;

      default:
        CHECK(false) << "Unknown field index: " << index;
        break;
    }
  }

  // Returns |index|-th field from LSB.
  uint64 get_field(int index) {
    switch (index) {
      case 0:
        return reg_.hot0_();

      case 1:
        return reg_.hot1_();

      case 2:
        return reg_.hot2_();

      case 3:
        return reg_.hot3_();

      default:
        LOG(FATAL) << "Unknown field index: " << index;
        unreachable();
    }
  }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 1> hot0_;
    platforms::darwinn::driver::Bitfield<1, 1> hot1_;
    platforms::darwinn::driver::Bitfield<2, 1> hot2_;
    platforms::darwinn::driver::Bitfield<3, 1> hot3_;
  } reg_;
};

// CSR helper to access fields for ScHostIntVector CSR.
class ScHostIntVector {
 public:
  ScHostIntVector() : ScHostIntVector(0ULL) {}
  explicit ScHostIntVector(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_vector0(uint64 value) { reg_.vector0_ = value; }
  uint64 vector0() const { return reg_.vector0_(); }
  void set_vector1(uint64 value) { reg_.vector1_ = value; }
  uint64 vector1() const { return reg_.vector1_(); }
  void set_vector2(uint64 value) { reg_.vector2_ = value; }
  uint64 vector2() const { return reg_.vector2_(); }
  void set_vector3(uint64 value) { reg_.vector3_ = value; }
  uint64 vector3() const { return reg_.vector3_(); }

  // Sets |index|-th field from LSB to |value|.
  void set_field(int index, uint64 value) {
    switch (index) {
      case 0:
        reg_.vector0_ = value;
        break;

      case 1:
        reg_.vector1_ = value;
        break;

      case 2:
        reg_.vector2_ = value;
        break;

      case 3:
        reg_.vector3_ = value;
        break;

      default:
        CHECK(false) << "Unknown field index: " << index;
        break;
    }
  }

  // Returns |index|-th field from LSB.
  uint64 get_field(int index) {
    switch (index) {
      case 0:
        return reg_.vector0_();

      case 1:
        return reg_.vector1_();

      case 2:
        return reg_.vector2_();

      case 3:
        return reg_.vector3_();

      default:
        LOG(FATAL) << "Unknown field index: " << index;
        unreachable();
    }
  }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 7> vector0_;
    platforms::darwinn::driver::Bitfield<7, 7> vector1_;
    platforms::darwinn::driver::Bitfield<14, 7> vector2_;
    platforms::darwinn::driver::Bitfield<21, 7> vector3_;
  } reg_;
};

// CSR helper to access fields for WireIntPendingBitArray and
// WireIntMaskArray CSR.
class WireIntBitArray {
 public:
  WireIntBitArray() : WireIntBitArray(0ULL) {}
  explicit WireIntBitArray(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_instruction_queue(uint64 value) { reg_.instruction_queue_ = value; }
  uint64 instruction_queue() const { return reg_.instruction_queue_(); }
  void set_input_actv_queue(uint64 value) { reg_.input_actv_queue_ = value; }
  uint64 input_actv_queue() const { return reg_.input_actv_queue_(); }
  void set_param_queue(uint64 value) { reg_.param_queue_ = value; }
  uint64 param_queue() const { return reg_.param_queue_(); }
  void set_output_actv_queue(uint64 value) { reg_.output_actv_queue_ = value; }
  uint64 output_actv_queue() const { return reg_.output_actv_queue_(); }
  void set_sc_host_0(uint64 value) { reg_.sc_host_0_ = value; }
  uint64 sc_host_0() const { return reg_.sc_host_0_(); }
  void set_sc_host_1(uint64 value) { reg_.sc_host_1_ = value; }
  uint64 sc_host_1() const { return reg_.sc_host_1_(); }
  void set_sc_host_2(uint64 value) { reg_.sc_host_2_ = value; }
  uint64 sc_host_2() const { return reg_.sc_host_2_(); }
  void set_sc_host_3(uint64 value) { reg_.sc_host_3_ = value; }
  uint64 sc_host_3() const { return reg_.sc_host_3_(); }
  void set_top_level_0(uint64 value) { reg_.top_level_0_ = value; }
  uint64 top_level_0() const { return reg_.top_level_0_(); }
  void set_top_level_1(uint64 value) { reg_.top_level_1_ = value; }
  uint64 top_level_1() const { return reg_.top_level_1_(); }
  void set_top_level_2(uint64 value) { reg_.top_level_2_ = value; }
  uint64 top_level_2() const { return reg_.top_level_2_(); }
  void set_top_level_3(uint64 value) { reg_.top_level_3_ = value; }
  uint64 top_level_3() const { return reg_.top_level_3_(); }
  void set_fatal_err(uint64 value) { reg_.fatal_err_ = value; }
  uint64 fatal_err() const { return reg_.fatal_err_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 1> instruction_queue_;
    platforms::darwinn::driver::Bitfield<1, 1> input_actv_queue_;
    platforms::darwinn::driver::Bitfield<2, 1> param_queue_;
    platforms::darwinn::driver::Bitfield<3, 1> output_actv_queue_;
    platforms::darwinn::driver::Bitfield<4, 1> sc_host_0_;
    platforms::darwinn::driver::Bitfield<5, 1> sc_host_1_;
    platforms::darwinn::driver::Bitfield<6, 1> sc_host_2_;
    platforms::darwinn::driver::Bitfield<7, 1> sc_host_3_;
    platforms::darwinn::driver::Bitfield<8, 1> top_level_0_;
    platforms::darwinn::driver::Bitfield<9, 1> top_level_1_;
    platforms::darwinn::driver::Bitfield<10, 1> top_level_2_;
    platforms::darwinn::driver::Bitfield<11, 1> top_level_3_;
    platforms::darwinn::driver::Bitfield<12, 1> fatal_err_;
  } reg_;
};

// Interface to access fields for tile configs.
class TileConfigInterface {
 public:
  virtual ~TileConfigInterface() = default;

  // Access to aggregated value.
  virtual void set_raw(uint64 value) = 0;
  virtual uint64 raw() const = 0;

  // Sets tile id.
  virtual void set_broadcast() = 0;
  virtual void set_tile(uint64 value) = 0;

  // Returns tile field.
  virtual uint64 tile() const = 0;
};

// Implements TileConfigInterface with given TILE_BITS.
template <int TILE_BITS>
class TileConfig : public TileConfigInterface {
 public:
  TileConfig() : TileConfig(0ULL) {}
  explicit TileConfig(uint64 value) { reg_.raw_ = value; }
  ~TileConfig() = default;

  void set_raw(uint64 value) override { reg_.raw_ = value; }
  uint64 raw() const override { return reg_.raw_; }

  void set_broadcast() override {
    reg_.tile_ = static_cast<uint64>(-1) & reg_.tile_.mask();
  }
  void set_tile(uint64 value) { reg_.tile_ = value; }
  uint64 tile() const override { return reg_.tile_(); }

 private:
  union {
    // Entire CSR value.
    uint64 raw_;
    // Tile id field.
    platforms::darwinn::driver::Bitfield<0, TILE_BITS> tile_;
  } reg_;
};

// CSR helper to access fields for clockEnableReg CSR.
class ClockEnableReg {
 public:
  ClockEnableReg() : ClockEnableReg(0ULL) {}
  explicit ClockEnableReg(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }
  void set_clock_enable(uint64 value) { reg_.clock_enable_ = value; }
  uint64 clock_enable() const { return reg_.clock_enable_(); }
  void set_idle_override(uint64 value) { reg_.idle_override_ = value; }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 1> clock_enable_;
    platforms::darwinn::driver::Bitfield<1, 1> idle_override_;
  } reg_;
};

// CSR helper to access fields for idleRegister CSR.
class IdleRegister {
 public:
  // Defaults to reset value.
  IdleRegister() : IdleRegister(0x00009000ULL) {}
  explicit IdleRegister(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_enable() { reg_.disable_idle_ = 0; }
  void set_disable() { reg_.disable_idle_ = 1; }
  void set_counter(uint64 value) { reg_.counter_ = value; }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 31> counter_;
    platforms::darwinn::driver::Bitfield<31, 1> disable_idle_;
  } reg_;
};

// CSR helper to access fields for logicShutdownPreReg/logicShutdownAllReg.
template <int NUM_BITS>
class ShutdownReg {
 public:
  // Defaults to reset value.
  ShutdownReg() : ShutdownReg(0x0) {
    set_logic_shutdown((1ULL << NUM_BITS) - 1);
  }
  explicit ShutdownReg(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_logic_shutdown(uint64 value) { reg_.logic_shutdown_ = value; }
  void set_logic_shutdown_ack(uint64 value) {
    reg_.logic_shutdown_ack_ = value;
  }
  uint64 logic_shutdown() const { return reg_.logic_shutdown_(); }
  uint64 logic_shutdown_ack() const { return reg_.logic_shutdown_ack_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, NUM_BITS> logic_shutdown_;
    platforms::darwinn::driver::Bitfield<NUM_BITS, NUM_BITS>
        logic_shutdown_ack_;
  } reg_;
};

// CSR helper to access fields for deepSleep.
class DeepSleep {
 public:
  // Defaults to reset value.
  DeepSleep() : DeepSleep(0x0) {}
  explicit DeepSleep(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_to_sleep_delay(uint64 value) { reg_.to_sleep_delay_ = value; }
  void set_to_wake_delay(uint64 value) { reg_.to_wake_delay_ = value; }
  uint64 narrow_mem_deep_sleep() const { return reg_.narrow_mem_deep_sleep_(); }
  uint64 wide_mem_deep_sleep() const { return reg_.wide_mem_deep_sleep_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 8> to_sleep_delay_;
    platforms::darwinn::driver::Bitfield<8, 8> to_wake_delay_;
    platforms::darwinn::driver::Bitfield<16, 1> narrow_mem_deep_sleep_;
    platforms::darwinn::driver::Bitfield<17, 1> wide_mem_deep_sleep_;
  } reg_;
};

// Implements field level access for SharedMemoryInitControl CSR.
class SharedMemoryInitControl {
 public:
  SharedMemoryInitControl() : SharedMemoryInitControl(/*value=*/0ULL) {}
  SharedMemoryInitControl(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_trigger(uint64 value) { reg_.trigger_ = value; }
  uint64 trigger() const { return reg_.trigger_(); }

  void set_run(uint64 value) { reg_.run_ = value; }
  uint64 run() const { return reg_.run_(); }

  void set_done(uint64 value) { reg_.done_ = value; }
  uint64 done() const { return reg_.done_(); }

 private:
  union {
    uint64 raw_;
    platforms::darwinn::driver::Bitfield<0, 1> trigger_;
    platforms::darwinn::driver::Bitfield<1, 1> run_;
    platforms::darwinn::driver::Bitfield<2, 1> done_;
  } reg_;
};

}  // namespace registers
}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_COMMON_CSR_HELPER_H_
