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

#ifndef DARWINN_DRIVER_CONFIG_BEAGLE_CSR_HELPER_H_
#define DARWINN_DRIVER_CONFIG_BEAGLE_CSR_HELPER_H_

#include "driver/bitfield.h"
#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {
namespace registers {

// CSR helper to access fields for omc0_d4 CSR.
class Omc0D4 {
 public:
  // Defaults to reset value.
  Omc0D4() : Omc0D4(0x1ULL) {}
  explicit Omc0D4(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_method_sel(uint64 value) { reg_.method_sel_ = value; }
  uint64 method_sel() const { return reg_.method_sel_(); }

  void set_thm_warn1(uint64 value) { reg_.thm_warn1_ = value; }
  uint64 thm_warn1() const { return reg_.thm_warn1_(); }

  void set_thm_warn_en(uint64 value) { reg_.thm_warn_en_ = value; }
  uint64 thm_warn_en() const { return reg_.thm_warn_en_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> method_sel_;
    platforms::darwinn::driver::Bitfield<1, 15> field_01_;
    platforms::darwinn::driver::Bitfield<16, 10> thm_warn1_;
    platforms::darwinn::driver::Bitfield<26, 5> field_26_;
    platforms::darwinn::driver::Bitfield<31, 1> thm_warn_en_;
  } reg_;
};

// CSR helper to access fields for omc0_d8 CSR.
class Omc0D8 {
 public:
  // Defaults to reset value.
  Omc0D8() : Omc0D8(0ULL) {}
  explicit Omc0D8(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_enbg(uint64 value) { reg_.enbg_ = value; }
  uint64 enbg() const { return reg_.enbg_(); }

  void set_envr(uint64 value) { reg_.envr_ = value; }
  uint64 envr() const { return reg_.envr_(); }

  void set_enad(uint64 value) { reg_.enad_ = value; }
  uint64 enad() const { return reg_.enad_(); }

  void set_thm_warn2(uint64 value) { reg_.thm_warn2_ = value; }
  uint64 thm_warn2() const { return reg_.thm_warn2_(); }

  void set_sd_en(uint64 value) { reg_.sd_en_ = value; }
  uint64 sd_en() const { return reg_.sd_en_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> enbg_;
    platforms::darwinn::driver::Bitfield<1, 1> envr_;
    platforms::darwinn::driver::Bitfield<2, 1> enad_;
    platforms::darwinn::driver::Bitfield<3, 13> field_03_;
    platforms::darwinn::driver::Bitfield<16, 10> thm_warn2_;
    platforms::darwinn::driver::Bitfield<26, 5> field_26_;
    platforms::darwinn::driver::Bitfield<31, 1> sd_en_;
  } reg_;
};

// CSR helper to access fields for omc0_dc CSR.
class Omc0DC {
 public:
  // Defaults to reset value.
  Omc0DC() : Omc0DC(0ULL) { set_data(0x3FF); }
  explicit Omc0DC(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_data(uint64 value) { reg_.data_ = value; }
  uint64 data() const { return reg_.data_(); }

  void set_sd_clear(uint64 value) { reg_.sd_clear_ = value; }
  uint64 sd_clear() const { return reg_.sd_clear_(); }

  void set_warn_clear(uint64 value) { reg_.warn_clear_ = value; }
  uint64 warn_clear() const { return reg_.warn_clear_(); }

  // Read-Only.
  uint64 sd_o() const { return reg_.sd_o_(); }
  uint64 warn_o() const { return reg_.warn_o_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> enthmc_;
    platforms::darwinn::driver::Bitfield<1, 15> field_01_;
    platforms::darwinn::driver::Bitfield<16, 10> data_;
    platforms::darwinn::driver::Bitfield<26, 2> field_26_;
    platforms::darwinn::driver::Bitfield<28, 1> sd_clear_;
    platforms::darwinn::driver::Bitfield<29, 1> warn_clear_;
    platforms::darwinn::driver::Bitfield<30, 1> sd_o_;
    platforms::darwinn::driver::Bitfield<31, 1> warn_o_;
  } reg_;
};

// CSR helper to access fields for rambist_ctrl_1 CSR.
class RamBistCtrl1 {
 public:
  // Defaults to reset value.
  RamBistCtrl1() : RamBistCtrl1(0ULL) {
    set_rg_rambist_gcbsel(0x1F);
    set_rg_rambist_topsel(0x3);
    set_rg_mbist_int_mask(0x7);
  }
  explicit RamBistCtrl1(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_rg_rambist_gcbsel(uint64 value) { reg_.rg_rambist_gcbsel_ = value; }
  uint64 rg_rambist_gcbsel() const { return reg_.rg_rambist_gcbsel_(); }

  void set_rg_rambist_topsel(uint64 value) { reg_.rg_rambist_topsel_ = value; }
  uint64 rg_rambist_topsel() const { return reg_.rg_rambist_topsel_(); }

  void set_rg_rambist_tckmode(uint64 value) {
    reg_.rg_rambist_tckmode_ = value;
  }
  uint64 rg_rambist_tckmode() const { return reg_.rg_rambist_tckmode_(); }

  void set_rg_rambist_req(uint64 value) { reg_.rg_rambist_req_ = value; }
  uint64 rg_rambist_req() const { return reg_.rg_rambist_req_(); }

  void set_rg_tck_invert(uint64 value) { reg_.rg_tck_invert_ = value; }
  uint64 rg_tck_invert() const { return reg_.rg_tck_invert_(); }

  void set_mbist_status(uint64 value) { reg_.mbist_status_ = value; }
  uint64 mbist_status() const { return reg_.mbist_status_(); }

  void set_rg_mbist_int_status(uint64 value) {
    reg_.rg_mbist_int_status_ = value;
  }
  uint64 rg_mbist_int_status() const { return reg_.rg_mbist_int_status_(); }

  void set_rg_mbist_int_mask(uint64 value) { reg_.rg_mbist_int_mask_ = value; }
  uint64 rg_mbist_int_mask() const { return reg_.rg_mbist_int_mask_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 5> rg_rambist_gcbsel_;
    platforms::darwinn::driver::Bitfield<5, 2> rg_rambist_topsel_;
    platforms::darwinn::driver::Bitfield<7, 1> field_07_;
    platforms::darwinn::driver::Bitfield<8, 1> rg_rambist_tckmode_;
    platforms::darwinn::driver::Bitfield<9, 1> rg_rambist_req_;
    platforms::darwinn::driver::Bitfield<10, 1> rg_tck_invert_;
    platforms::darwinn::driver::Bitfield<11, 1> field_11_;
    platforms::darwinn::driver::Bitfield<12, 2> mbist_status_;
    platforms::darwinn::driver::Bitfield<14, 2> field_14_;
    platforms::darwinn::driver::Bitfield<16, 3> rg_mbist_int_status_;
    platforms::darwinn::driver::Bitfield<19, 1> field_19_;
    platforms::darwinn::driver::Bitfield<20, 3> rg_mbist_int_mask_;
    platforms::darwinn::driver::Bitfield<23, 9> field_23_;
  } reg_;
};

// CSR helper to access fields for efuse_00 CSR.
class Efuse00 {
 public:
  // Defaults to reset value.
  Efuse00() : Efuse00(0ULL) {}
  explicit Efuse00(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_ef_int_mask(uint64 value) { reg_.ef_int_mask_ = value; }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> ef_single_step_dis_;
    platforms::darwinn::driver::Bitfield<1, 2> ef_prod_sel_;
    platforms::darwinn::driver::Bitfield<3, 3> ef_refclk_sel_ovr_;
    platforms::darwinn::driver::Bitfield<6, 1> ef_pcie_gen1_link_;
    platforms::darwinn::driver::Bitfield<7, 1> ef_usb_ssc_mode_0_;
    platforms::darwinn::driver::Bitfield<8, 5> ef_i2caddr_ovr_;
    platforms::darwinn::driver::Bitfield<13, 3> ef_psigma_;
    platforms::darwinn::driver::Bitfield<16, 1> ef_mbist_dis_;
    platforms::darwinn::driver::Bitfield<17, 1> ef_w_dis_;
    platforms::darwinn::driver::Bitfield<18, 1> ef_thm_int_mask_;
    platforms::darwinn::driver::Bitfield<19, 1> ef_int_mask_;
    platforms::darwinn::driver::Bitfield<20, 2> ef_pwr_state_dis_;
    platforms::darwinn::driver::Bitfield<22, 1> ef_usb_ssc_mode_1_;
    platforms::darwinn::driver::Bitfield<23, 1> ef_8051_rom_500m_;
    platforms::darwinn::driver::Bitfield<24, 8> ef_pll_M_;
  } reg_;
};

// CSR helper to access fields for scu_ctrl_0 CSR.
class ScuCtrl0 {
 public:
  // Defaults to reset value.
  ScuCtrl0() : ScuCtrl0(0ULL) {
    set_rg_pllclk_sel(1);
    set_rg_usb_slp_phy_mode(1);
    set_rg_pcie_inact_phy_mode(1);
    set_rg_usb_inact_phy_mode(1);
  }
  explicit ScuCtrl0(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_rg_pllclk_sel(uint64 value) { reg_.rg_pllclk_sel_ = value; }
  uint64 rg_pllclk_sel() const { return reg_.rg_pllclk_sel_(); }

  void set_rg_usb_slp_phy_mode(uint64 value) {
    reg_.rg_usb_slp_phy_mode_ = value;
  }
  uint64 rg_usb_slp_phy_mode() const { return reg_.rg_usb_slp_phy_mode_(); }

  void set_rg_pcie_inact_phy_mode(uint64 value) {
    reg_.rg_pcie_inact_phy_mode_ = value;
  }
  uint64 rg_pcie_inact_phy_mode() const {
    return reg_.rg_pcie_inact_phy_mode_();
  }

  void set_rg_usb_inact_phy_mode(uint64 value) {
    reg_.rg_usb_inact_phy_mode_ = value;
  }
  uint64 rg_usb_inact_phy_mode() const { return reg_.rg_usb_inact_phy_mode_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> rg_pllclk_sel_;
    platforms::darwinn::driver::Bitfield<1, 1> rg_single_exit_;
    platforms::darwinn::driver::Bitfield<2, 1> rg_single_link_rstn_;
    platforms::darwinn::driver::Bitfield<3, 1> rg_sleep_chk_idle_;
    platforms::darwinn::driver::Bitfield<4, 2> rg_pcie_slp_phy_mode_;
    platforms::darwinn::driver::Bitfield<6, 2> rg_usb_slp_phy_mode_;
    platforms::darwinn::driver::Bitfield<8, 3> rg_pcie_inact_phy_mode_;
    platforms::darwinn::driver::Bitfield<11, 3> rg_usb_inact_phy_mode_;
    platforms::darwinn::driver::Bitfield<14, 2> rg_mem_mode_dis_;
    platforms::darwinn::driver::Bitfield<16, 1> rg_phy_prg_;
    platforms::darwinn::driver::Bitfield<17, 1> bt_phy_prg_;
    platforms::darwinn::driver::Bitfield<18, 1> bt_vbus_sel_;
    platforms::darwinn::driver::Bitfield<19, 1> bt_bus_pwr_;
  } reg_;
};

// CSR helper to access fields for scu_ctrl_2 CSR.
class ScuCtrl2 {
 public:
  // Defaults to reset value.
  ScuCtrl2() : ScuCtrl2(0ULL) {}
  explicit ScuCtrl2(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_rg_gated_gcb(uint64 value) { reg_.rg_gated_gcb_ = value; }
  uint64 rg_gated_gcb() const { return reg_.rg_gated_gcb_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> rg_rst_pcie_;
    platforms::darwinn::driver::Bitfield<1, 1> rg_rst_pcie_axi_;
    platforms::darwinn::driver::Bitfield<2, 2> rg_rst_gcb_;
    platforms::darwinn::driver::Bitfield<4, 1> rg_rst_pcieslv_abm_;
    platforms::darwinn::driver::Bitfield<5, 1> rg_rst_pciemst_abm_;
    platforms::darwinn::driver::Bitfield<6, 1> rg_rst_omc_;
    platforms::darwinn::driver::Bitfield<7, 1> rg_rst_mbist_;
    platforms::darwinn::driver::Bitfield<8, 1> rg_rst_usb_;
    platforms::darwinn::driver::Bitfield<9, 1> rg_rst_usb_subsys_;
    platforms::darwinn::driver::Bitfield<10, 1> rg_rst_full_;
    platforms::darwinn::driver::Bitfield<11, 1> rg_rst_link_;
    platforms::darwinn::driver::Bitfield<12, 1> rg_rst_i2c_;
    platforms::darwinn::driver::Bitfield<13, 1> rg_rst_scu_;
    platforms::darwinn::driver::Bitfield<14, 1> rg_self_rst_subsys_;
    platforms::darwinn::driver::Bitfield<15, 1> rg_rst_brg_;
    platforms::darwinn::driver::Bitfield<16, 1> rg_gated_pcie_;
    platforms::darwinn::driver::Bitfield<17, 1> rg_gated_phy_cfg_;
    platforms::darwinn::driver::Bitfield<18, 2> rg_gated_gcb_;
    platforms::darwinn::driver::Bitfield<20, 1> rg_gated_pcieslv_abm_;
    platforms::darwinn::driver::Bitfield<21, 1> rg_gated_pciemst_abm_;
    platforms::darwinn::driver::Bitfield<22, 1> rg_gated_omc_;
    platforms::darwinn::driver::Bitfield<23, 1> rg_gated_mbist_;
    platforms::darwinn::driver::Bitfield<24, 1> rg_gated_usb_;
    platforms::darwinn::driver::Bitfield<25, 1> rg_gated_usb_subsys_;
    platforms::darwinn::driver::Bitfield<26, 1> rg_gated_8051_;
  } reg_;
};

// CSR helper to access fields for scu_ctrl_3 CSR.
class ScuCtrl3 {
 public:
  enum class GcbClock {
    k63MHZ,
    k125MHZ,
    k250MHZ,
    k500MHZ,
  };
  enum class AxiClock {
    k125MHZ,
    k250MHZ,
  };
  enum class Usb8051Clock {
    k250MHZ,
    k500MHZ,
  };

  // Defaults to reset value.
  ScuCtrl3() : ScuCtrl3(0x80050410ULL) {}
  explicit ScuCtrl3(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_rg_force_sleep(uint64 value) { reg_.rg_force_sleep_ = value; }
  uint64 rg_force_sleep() const { return reg_.rg_force_sleep_(); }

  void set_cur_pwr_state(uint64 value) { reg_.cur_pwr_state_ = value; }
  uint64 cur_pwr_state() const { return reg_.cur_pwr_state_(); }

  void set_gcb_clock_rate(GcbClock rate) {
    switch (rate) {
      case GcbClock::k63MHZ:
        reg_.rg_gcb_clkdiv_ = 3;
        break;
      case GcbClock::k125MHZ:
        reg_.rg_gcb_clkdiv_ = 2;
        break;
      case GcbClock::k250MHZ:
        reg_.rg_gcb_clkdiv_ = 1;
        break;
      case GcbClock::k500MHZ:
        reg_.rg_gcb_clkdiv_ = 0;
        break;
    }
  }

  GcbClock gcb_clock_rate() const {
    switch (reg_.rg_gcb_clkdiv_()) {
      case 3:
        return GcbClock::k63MHZ;
      case 2:
        return GcbClock::k125MHZ;
      case 1:
        return GcbClock::k250MHZ;
      default:
        return GcbClock::k500MHZ;
    }
  }

  void set_axi_clock_rate(AxiClock rate) {
    switch (rate) {
      case AxiClock::k125MHZ:
        reg_.rg_axi_clk_125m_ = 1;
        break;
      case AxiClock::k250MHZ:
        reg_.rg_axi_clk_125m_ = 0;
        break;
    }
  }

  AxiClock axi_clock_rate() const {
    if (reg_.rg_axi_clk_125m_()) {
      return AxiClock::k125MHZ;
    } else {
      return AxiClock::k250MHZ;
    }
  }

  void set_usb_8051_clock_rate(Usb8051Clock rate) {
    switch (rate) {
      case Usb8051Clock::k250MHZ:
        reg_.rg_8051_clk_250m_ = 1;
        break;
      case Usb8051Clock::k500MHZ:
        reg_.rg_8051_clk_250m_ = 0;
        break;
    }
  }

  Usb8051Clock usb_8051_clock_rate() const {
    if (reg_.rg_8051_clk_250m_()) {
      return Usb8051Clock::k250MHZ;
    } else {
      return Usb8051Clock::k500MHZ;
    }
  }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 1> pcie_state_l1p2_;
    platforms::darwinn::driver::Bitfield<1, 1> pcie_state_l0s_;
    platforms::darwinn::driver::Bitfield<2, 1> pcie_state_l0_;
    platforms::darwinn::driver::Bitfield<3, 1> cur_gated_gcb_;
    platforms::darwinn::driver::Bitfield<4, 1> cur_rst_gcb_;
    platforms::darwinn::driver::Bitfield<5, 1> field_05_;
    platforms::darwinn::driver::Bitfield<6, 1> cur_gcb_sram_sd_;
    platforms::darwinn::driver::Bitfield<7, 1> cur_gcb_sram_dslp_;
    platforms::darwinn::driver::Bitfield<8, 2> cur_pwr_state_;
    platforms::darwinn::driver::Bitfield<10, 2> pcie_gen_info_;
    platforms::darwinn::driver::Bitfield<12, 2> rg_force_ram_dslp_;
    platforms::darwinn::driver::Bitfield<14, 2> rg_force_ram_sd_;
    platforms::darwinn::driver::Bitfield<16, 3> rg_sd2wk_dly_;
    platforms::darwinn::driver::Bitfield<19, 1> rg_slp_mode_req_;
    platforms::darwinn::driver::Bitfield<20, 2> rg_force_inact_;
    platforms::darwinn::driver::Bitfield<22, 2> rg_force_sleep_;
    platforms::darwinn::driver::Bitfield<24, 1> field_24_;
    platforms::darwinn::driver::Bitfield<25, 1> rg_link_rdy_ovr_;
    platforms::darwinn::driver::Bitfield<26, 2> rg_pwr_state_ovr_;
    platforms::darwinn::driver::Bitfield<28, 2> rg_gcb_clkdiv_;
    platforms::darwinn::driver::Bitfield<30, 1> rg_axi_clk_125m_;
    platforms::darwinn::driver::Bitfield<31, 1> rg_8051_clk_250m_;
  } reg_;
};

// CSR helper to access fields for scu_ctrl_6 CSR.
class ScuCtrl6 {
 public:
  // Defaults to reset value.
  ScuCtrl6() : ScuCtrl6(0ULL) {}
  explicit ScuCtrl6(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_rg_gcb_spare_in(uint64 value) { reg_.rg_gcb_spare_in_ = value; }
  uint64 rg_gcb_spare_in() const { return reg_.rg_gcb_spare_in_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 2> rg_pad_ds_;
    platforms::darwinn::driver::Bitfield<2, 2> rg_pad_ds_i2c_;
    platforms::darwinn::driver::Bitfield<4, 2> rg_pad_ds_gpio_;
    platforms::darwinn::driver::Bitfield<6, 2> rg_pad_ds_xin_;
    platforms::darwinn::driver::Bitfield<8, 8> rg_pinmux_sel_;
    platforms::darwinn::driver::Bitfield<16, 4> rg_gcb_spare_in_;
    platforms::darwinn::driver::Bitfield<20, 4> gcb_spare_out_;
    platforms::darwinn::driver::Bitfield<24, 1> warning_o_;
    platforms::darwinn::driver::Bitfield<25, 1> int_mbist_;
    platforms::darwinn::driver::Bitfield<26, 1> err_resp_isr_0_;
    platforms::darwinn::driver::Bitfield<27, 1> err_resp_isr_1_;
    platforms::darwinn::driver::Bitfield<28, 2> rg_jtag_sel_;
    platforms::darwinn::driver::Bitfield<30, 1> rg_jtag_io_sel_;
    platforms::darwinn::driver::Bitfield<31, 1> field_31_;
  } reg_;
};

// CSR helper to access fields for scu_ctr_7 CSR.
class ScuCtrl7 {
 public:
  // Defaults to reset value.
  ScuCtrl7() : ScuCtrl7(0ULL) {
    set_rg_inact_thd(0x3F);
    set_rg_boot_failure_mask(0x3);
  }
  explicit ScuCtrl7(uint64 value) { reg_.raw_ = value; }

  void set_raw(uint64 value) { reg_.raw_ = value; }
  uint64 raw() const { return reg_.raw_; }

  void set_rg_boot_failure_mask(uint64 value) {
    reg_.rg_boot_failure_mask_ = value;
  }
  uint64 rg_boot_failure_mask() const { return reg_.rg_boot_failure_mask_(); }

  void set_rg_inact_thd(uint64 value) { reg_.rg_inact_thd_ = value; }
  uint64 rg_inact_thd() const { return reg_.rg_inact_thd_(); }

  void set_rg_boot_failure_raw(uint64 value) {
    reg_.rg_boot_failure_raw_ = value;
  }
  uint64 rg_boot_failure_raw() const { return reg_.rg_boot_failure_raw_(); }

  void set_pll_lock_failure(uint64 value) { reg_.pll_lock_failure_ = value; }
  uint64 pll_lock_failure() const { return reg_.pll_lock_failure_(); }

  void set_usb_sel_failure(uint64 value) { reg_.usb_sel_failure_ = value; }
  uint64 usb_sel_failure() const { return reg_.usb_sel_failure_(); }

 private:
  union {
    uint64 raw_;
    // These are named after fields in the spec.
    platforms::darwinn::driver::Bitfield<0, 16> rg_inact_thd_;
    platforms::darwinn::driver::Bitfield<16, 1> pll_lock_failure_;
    platforms::darwinn::driver::Bitfield<17, 1> usb_sel_failure_;
    platforms::darwinn::driver::Bitfield<18, 2> rg_boot_failure_mask_;
    platforms::darwinn::driver::Bitfield<20, 2> rg_boot_failure_raw_;
    platforms::darwinn::driver::Bitfield<22, 10> field_22_;
  } reg_;
};

}  // namespace registers
}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_BEAGLE_CSR_HELPER_H_
