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

#include "driver/beagle/beagle_top_level_interrupt_manager.h"

#include <utility>

#include "driver/config/beagle_csr_helper.h"
#include "driver/interrupt/interrupt_controller_interface.h"
#include "driver/registers/registers.h"
#include "port/errors.h"
#include "port/integral_types.h"
#include "port/logging.h"
#include "port/status_macros.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace {

// Top Level Interrupt ids:
// https://g3doc.corp.google.com/platforms/darwinn/silo/g3doc/spec/index.md#interrupt-handling
constexpr int kThermalShutdownId = 0;
constexpr int kPcieErrorId = 1;
constexpr int kMbistId = 2;
constexpr int kThermalWarningId = 3;

}  // namespace

BeagleTopLevelInterruptManager::BeagleTopLevelInterruptManager(
    std::unique_ptr<InterruptControllerInterface> interrupt_controller,
    const config::ChipConfig& config, Registers* registers)
    : TopLevelInterruptManager(std::move(interrupt_controller)),
      apex_csr_offsets_(config.GetApexCsrOffsets()),
      scu_csr_offsets_(config.GetScuCsrOffsets()),
      registers_(registers) {
  CHECK(registers != nullptr);
}

util::Status BeagleTopLevelInterruptManager::DoEnableInterrupts() {
  RETURN_IF_ERROR(EnableThermalWarningInterrupt());
  RETURN_IF_ERROR(EnableMbistInterrupt());
  RETURN_IF_ERROR(EnablePcieErrorInterrupt());
  RETURN_IF_ERROR(EnableThermalShutdownInterrupt());
  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::DoDisableInterrupts() {
  RETURN_IF_ERROR(DisableThermalWarningInterrupt());
  RETURN_IF_ERROR(DisableMbistInterrupt());
  RETURN_IF_ERROR(DisablePcieErrorInterrupt());
  RETURN_IF_ERROR(DisableThermalShutdownInterrupt());
  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::DoHandleInterrupt(int id) {
  switch (id) {
    case kThermalWarningId:
      return HandleThermalWarningInterrupt();

    case kMbistId:
      return HandleMbistInterrupt();

    case kPcieErrorId:
      return HandlePcieErrorInterrupt();

    case kThermalShutdownId:
      return HandleThermalShutdownInterrupt();

    default:
      return util::InvalidArgumentError(
          StringPrintf("Unknown top level id: %d", id));
  }
}

util::Status BeagleTopLevelInterruptManager::EnableThermalWarningInterrupt() {
  // 1. Enable thermal warning through omc0_d4.
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 omc0_d4_read,
                   registers_->Read32(apex_csr_offsets_.omc0_d4));
  driver::config::registers::Omc0D4 omc0_d4_helper(omc0_d4_read);
  omc0_d4_helper.set_thm_warn_en(1);

  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.omc0_d4, omc0_d4_helper.raw()));

  // 2. Set thermal warning threshold temperature.
  // TODO: This is important in the real chip, but not for DV purposes
  // now.

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::EnableMbistInterrupt() {
  // 1. Unmask interrupts, and clear interrupt status.
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 rambist_ctrl_1_read,
                   registers_->Read32(apex_csr_offsets_.rambist_ctrl_1));
  driver::config::registers::RamBistCtrl1 ram_bist_ctrl_1_helper(
      rambist_ctrl_1_read);
  // rg_mbist_int_status is write 1 to clear. Set it to 0 not to clear it.
  ram_bist_ctrl_1_helper.set_rg_mbist_int_status(0);
  ram_bist_ctrl_1_helper.set_rg_mbist_int_mask(0);

  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.rambist_ctrl_1,
                                      ram_bist_ctrl_1_helper.raw()));

  // 2. Unmask interrupts, and clear interrupt status in scu_ctr_7.
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 scu_ctr_7_read,
                   registers_->Read32(scu_csr_offsets_.scu_ctr_7));
  driver::config::registers::ScuCtrl7 scu7_helper(scu_ctr_7_read);
  // pll_lock_failure, and usb_sel_failure are write 1 to clear. Set them to 0
  // not to clear them.
  scu7_helper.set_pll_lock_failure(0);
  scu7_helper.set_usb_sel_failure(0);
  scu7_helper.set_rg_boot_failure_mask(0);

  RETURN_IF_ERROR(
      registers_->Write32(scu_csr_offsets_.scu_ctr_7, scu7_helper.raw()));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::EnablePcieErrorInterrupt() {
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.slv_abm_en, 1));
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.mst_abm_en, 1));
  // Write 0x3 to unmask.
  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.slv_err_resp_isr_mask, 0x3));
  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.mst_err_resp_isr_mask, 0x3));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::EnableThermalShutdownInterrupt() {
  // 1. Enable thermal shutdown through omc0_d8.
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 omc0_d8_read,
                   registers_->Read32(apex_csr_offsets_.omc0_d8));
  driver::config::registers::Omc0D8 omc0_d8_helper(omc0_d8_read);
  omc0_d8_helper.set_sd_en(1);

  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.omc0_d8, omc0_d8_helper.raw()));

  // 2. Set thermal shutdown threshold temperature.
  // TODO: This is important in the real chip, but not for DV purposes
  // now.

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::DisableThermalWarningInterrupt() {
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 omc0_d4_read,
                   registers_->Read32(apex_csr_offsets_.omc0_d4));
  driver::config::registers::Omc0D4 omc0_d4_helper(omc0_d4_read);
  omc0_d4_helper.set_thm_warn_en(0);

  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.omc0_d4, omc0_d4_helper.raw()));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::DisableMbistInterrupt() {
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 rambist_ctrl_1_read,
                   registers_->Read32(apex_csr_offsets_.rambist_ctrl_1));
  driver::config::registers::RamBistCtrl1 ram_bist_ctrl_1_helper(
      rambist_ctrl_1_read);
  ram_bist_ctrl_1_helper.set_rg_mbist_int_mask(0x7);

  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.rambist_ctrl_1,
                                      ram_bist_ctrl_1_helper.raw()));

  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 scu_ctr_7_read,
                   registers_->Read32(scu_csr_offsets_.scu_ctr_7));
  driver::config::registers::ScuCtrl7 scu7_helper(scu_ctr_7_read);
  scu7_helper.set_rg_boot_failure_mask(0x3);

  RETURN_IF_ERROR(
      registers_->Write32(scu_csr_offsets_.scu_ctr_7, scu7_helper.raw()));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::DisablePcieErrorInterrupt() {
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.slv_abm_en, 0));
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.mst_abm_en, 0));
  // Write 0x0 to mask.
  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.slv_err_resp_isr_mask, 0));
  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.mst_err_resp_isr_mask, 0));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::DisableThermalShutdownInterrupt() {
  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 omc0_d8_read,
                   registers_->Read32(apex_csr_offsets_.omc0_d8));
  driver::config::registers::Omc0D8 omc0_d8_helper(omc0_d8_read);
  omc0_d8_helper.set_sd_en(0);

  RETURN_IF_ERROR(
      registers_->Write32(apex_csr_offsets_.omc0_d8, omc0_d8_helper.raw()));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::HandleThermalWarningInterrupt() {
  // Read register to preserve other fields. Also, warn_o field needs to be read
  // before clearing warn_clear, i.e. read before write field.
  ASSIGN_OR_RETURN(const uint32 omc0_dc_read,
                   registers_->Read32(apex_csr_offsets_.omc0_dc));
  driver::config::registers::Omc0DC helper(omc0_dc_read);

  // Unconditionally clears interrupts. Proper interrupt management has to
  // handle the thermal warning and wait for temperature to go down below
  // threshold before re-enabling.
  if (helper.warn_o()) {
    VLOG(5) << "Thermal warning interrupt received";
    helper.set_warn_clear(1);  // Writes 1 to clear.
  }
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.omc0_dc, helper.raw()));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::HandleMbistInterrupt() {
  ASSIGN_OR_RETURN(const uint32 rambist_ctrl_1_read,
                   registers_->Read32(apex_csr_offsets_.rambist_ctrl_1));
  driver::config::registers::RamBistCtrl1 ram_bist_ctrl_1_helper(
      rambist_ctrl_1_read);

  // Proper interrupt management is required in the real chip. For DV, just
  // print whether we received the correct interrupt.
  uint64 status_value = 0x0;
  constexpr uint64 kMbistFail = 0x1;
  if ((ram_bist_ctrl_1_helper.rg_mbist_int_status() & kMbistFail) ==
      kMbistFail) {
    VLOG(5) << "Mbist fail interrupt received";
    status_value |= kMbistFail;
  }

  constexpr uint64 kMbistTimeout = 0x2;
  if ((ram_bist_ctrl_1_helper.rg_mbist_int_status() & kMbistTimeout) ==
      kMbistTimeout) {
    VLOG(5) << "Mbist timeout interrupt received";
    status_value |= kMbistTimeout;
  }

  constexpr uint64 kMbistFinish = 0x4;
  if ((ram_bist_ctrl_1_helper.rg_mbist_int_status() & kMbistFinish) ==
      kMbistFinish) {
    VLOG(5) << "Mbist finish interrupt received";
    status_value |= kMbistFinish;
  }

  ram_bist_ctrl_1_helper.set_rg_mbist_int_status(status_value);
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.rambist_ctrl_1,
                                      ram_bist_ctrl_1_helper.raw()));

  // Read register to preserve other fields.
  ASSIGN_OR_RETURN(const uint32 scu_ctr_7_read,
                   registers_->Read32(scu_csr_offsets_.scu_ctr_7));
  driver::config::registers::ScuCtrl7 scu7_helper(scu_ctr_7_read);

  // Proper interrupt management is required in the real chip. For DV, just
  // print whether we received the correct interrupt.
  if (scu7_helper.usb_sel_failure()) {
    VLOG(5) << "bt_usb_sel violates the eFuse interrupt received";
    scu7_helper.set_usb_sel_failure(1);
  }
  if (scu7_helper.pll_lock_failure()) {
    VLOG(5) << "PLL lock timeout interrupt received";
    scu7_helper.set_pll_lock_failure(1);
  }

  RETURN_IF_ERROR(
      registers_->Write32(scu_csr_offsets_.scu_ctr_7, scu7_helper.raw()));

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::HandlePcieErrorInterrupt() {
  // Disable and enable abm_en to handle interrupts.
  ASSIGN_OR_RETURN(const uint32 slave_write_error,
                   registers_->Read32(apex_csr_offsets_.slv_wr_err_resp));
  if (slave_write_error == 1) {
    VLOG(5) << "Slave write interrupt received";
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.slv_abm_en, 0));
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.slv_abm_en, 1));
  }
  ASSIGN_OR_RETURN(const uint32 slave_read_error,
                   registers_->Read32(apex_csr_offsets_.slv_rd_err_resp));
  if (slave_read_error == 1) {
    VLOG(5) << "Slave read interrupt received";
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.slv_abm_en, 0));
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.slv_abm_en, 1));
  }

  ASSIGN_OR_RETURN(const uint32 master_write_error,
                   registers_->Read32(apex_csr_offsets_.mst_wr_err_resp));
  if (master_write_error == 1) {
    VLOG(5) << "Master write interrupt received";
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.mst_abm_en, 0));
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.mst_abm_en, 1));
  }
  ASSIGN_OR_RETURN(const uint32 master_read_error,
                   registers_->Read32(apex_csr_offsets_.mst_rd_err_resp));
  if (master_read_error == 1) {
    VLOG(5) << "Master read interrupt received";
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.mst_abm_en, 0));
    RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.mst_abm_en, 1));
  }

  return util::Status();  // OK
}

util::Status BeagleTopLevelInterruptManager::HandleThermalShutdownInterrupt() {
  // Read register to preserve other fields. Also, sd_o field needs to be read
  // before clearing sd_clear, i.e. read before write field.
  ASSIGN_OR_RETURN(const uint32 omc0_dc_read,
                   registers_->Read32(apex_csr_offsets_.omc0_dc));
  driver::config::registers::Omc0DC helper(omc0_dc_read);

  // Unconditionally clears interrupts. Proper interrupt management has to
  // handle the thermal shutdown, and wait for temperature to go down below
  // threshold before re-enabling.
  if (helper.sd_o()) {
    VLOG(5) << "Thermal shutdown interrupt received";
    helper.set_sd_clear(1);  // Writes 1 clear.
  }
  RETURN_IF_ERROR(registers_->Write32(apex_csr_offsets_.omc0_dc, helper.raw()));

  return util::Status();  // OK
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
