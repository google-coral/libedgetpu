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

#ifndef DARWINN_DRIVER_CONFIG_DEBUG_HIB_USER_CSR_OFFSETS_H_
#define DARWINN_DRIVER_CONFIG_DEBUG_HIB_USER_CSR_OFFSETS_H_

#include "port/integral_types.h"

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for user hib. Members are intentionally named to match the
// GCSR register names.
struct DebugHibUserCsrOffsets {
  uint64 instruction_inbound_queue_total_occupancy;
  uint64 instruction_inbound_queue_threshold_counter;
  uint64 instruction_inbound_queue_insertion_counter;
  uint64 instruction_inbound_queue_full_counter;
  uint64 input_actv_inbound_queue_total_occupancy;
  uint64 input_actv_inbound_queue_threshold_counter;
  uint64 input_actv_inbound_queue_insertion_counter;
  uint64 input_actv_inbound_queue_full_counter;
  uint64 param_inbound_queue_total_occupancy;
  uint64 param_inbound_queue_threshold_counter;
  uint64 param_inbound_queue_insertion_counter;
  uint64 param_inbound_queue_full_counter;
  uint64 output_actv_inbound_queue_total_occupancy;
  uint64 output_actv_inbound_queue_threshold_counter;
  uint64 output_actv_inbound_queue_insertion_counter;
  uint64 output_actv_inbound_queue_full_counter;
  uint64 status_block_write_inbound_queue_total_occupancy;
  uint64 status_block_write_inbound_queue_threshold_counter;
  uint64 status_block_write_inbound_queue_insertion_counter;
  uint64 status_block_write_inbound_queue_full_counter;
  uint64 queue_fetch_inbound_queue_total_occupancy;
  uint64 queue_fetch_inbound_queue_threshold_counter;
  uint64 queue_fetch_inbound_queue_insertion_counter;
  uint64 queue_fetch_inbound_queue_full_counter;
  uint64 instruction_outbound_queue_total_occupancy;
  uint64 instruction_outbound_queue_threshold_counter;
  uint64 instruction_outbound_queue_insertion_counter;
  uint64 instruction_outbound_queue_full_counter;
  uint64 input_actv_outbound_queue_total_occupancy;
  uint64 input_actv_outbound_queue_threshold_counter;
  uint64 input_actv_outbound_queue_insertion_counter;
  uint64 input_actv_outbound_queue_full_counter;
  uint64 param_outbound_queue_total_occupancy;
  uint64 param_outbound_queue_threshold_counter;
  uint64 param_outbound_queue_insertion_counter;
  uint64 param_outbound_queue_full_counter;
  uint64 output_actv_outbound_queue_total_occupancy;
  uint64 output_actv_outbound_queue_threshold_counter;
  uint64 output_actv_outbound_queue_insertion_counter;
  uint64 output_actv_outbound_queue_full_counter;
  uint64 status_block_write_outbound_queue_total_occupancy;
  uint64 status_block_write_outbound_queue_threshold_counter;
  uint64 status_block_write_outbound_queue_insertion_counter;
  uint64 status_block_write_outbound_queue_full_counter;
  uint64 queue_fetch_outbound_queue_total_occupancy;
  uint64 queue_fetch_outbound_queue_threshold_counter;
  uint64 queue_fetch_outbound_queue_insertion_counter;
  uint64 queue_fetch_outbound_queue_full_counter;
  uint64 page_table_request_outbound_queue_total_occupancy;
  uint64 page_table_request_outbound_queue_threshold_counter;
  uint64 page_table_request_outbound_queue_insertion_counter;
  uint64 page_table_request_outbound_queue_full_counter;
  uint64 read_tracking_fifo_total_occupancy;
  uint64 read_tracking_fifo_threshold_counter;
  uint64 read_tracking_fifo_insertion_counter;
  uint64 read_tracking_fifo_full_counter;
  uint64 write_tracking_fifo_total_occupancy;
  uint64 write_tracking_fifo_threshold_counter;
  uint64 write_tracking_fifo_insertion_counter;
  uint64 write_tracking_fifo_full_counter;
  uint64 read_buffer_total_occupancy;
  uint64 read_buffer_threshold_counter;
  uint64 read_buffer_insertion_counter;
  uint64 read_buffer_full_counter;
  uint64 axi_aw_credit_shim_total_occupancy;
  uint64 axi_aw_credit_shim_threshold_counter;
  uint64 axi_aw_credit_shim_insertion_counter;
  uint64 axi_aw_credit_shim_full_counter;
  uint64 axi_ar_credit_shim_total_occupancy;
  uint64 axi_ar_credit_shim_threshold_counter;
  uint64 axi_ar_credit_shim_insertion_counter;
  uint64 axi_ar_credit_shim_full_counter;
  uint64 axi_w_credit_shim_total_occupancy;
  uint64 axi_w_credit_shim_threshold_counter;
  uint64 axi_w_credit_shim_insertion_counter;
  uint64 axi_w_credit_shim_full_counter;
  uint64 instruction_inbound_queue_empty_cycles_count;
  uint64 input_actv_inbound_queue_empty_cycles_count;
  uint64 param_inbound_queue_empty_cycles_count;
  uint64 output_actv_inbound_queue_empty_cycles_count;
  uint64 status_block_write_inbound_queue_empty_cycles_count;
  uint64 queue_fetch_inbound_queue_empty_cycles_count;
  uint64 instruction_outbound_queue_empty_cycles_count;
  uint64 input_actv_outbound_queue_empty_cycles_count;
  uint64 param_outbound_queue_empty_cycles_count;
  uint64 output_actv_outbound_queue_empty_cycles_count;
  uint64 status_block_write_outbound_queue_empty_cycles_count;
  uint64 queue_fetch_outbound_queue_empty_cycles_count;
  uint64 page_table_request_outbound_queue_empty_cycles_count;
  uint64 read_tracking_fifo_empty_cycles_count;
  uint64 write_tracking_fifo_empty_cycles_count;
  uint64 read_buffer_empty_cycles_count;
  uint64 read_request_arbiter_instruction_request_cycles;
  uint64 read_request_arbiter_instruction_blocked_cycles;
  uint64 read_request_arbiter_instruction_blocked_by_arbitration_cycles;
  uint64 read_request_arbiter_instruction_cycles_blocked_over_threshold;
  uint64 read_request_arbiter_input_actv_request_cycles;
  uint64 read_request_arbiter_input_actv_blocked_cycles;
  uint64 read_request_arbiter_input_actv_blocked_by_arbitration_cycles;
  uint64 read_request_arbiter_input_actv_cycles_blocked_over_threshold;
  uint64 read_request_arbiter_param_request_cycles;
  uint64 read_request_arbiter_param_blocked_cycles;
  uint64 read_request_arbiter_param_blocked_by_arbitration_cycles;
  uint64 read_request_arbiter_param_cycles_blocked_over_threshold;
  uint64 read_request_arbiter_queue_fetch_request_cycles;
  uint64 read_request_arbiter_queue_fetch_blocked_cycles;
  uint64 read_request_arbiter_queue_fetch_blocked_by_arbitration_cycles;
  uint64 read_request_arbiter_queue_fetch_cycles_blocked_over_threshold;
  uint64 read_request_arbiter_page_table_request_request_cycles;
  uint64 read_request_arbiter_page_table_request_blocked_cycles;
  uint64 read_request_arbiter_page_table_request_blocked_by_arbitration_cycles;
  uint64 read_request_arbiter_page_table_request_cycles_blocked_over_threshold;
  uint64 write_request_arbiter_output_actv_request_cycles;
  uint64 write_request_arbiter_output_actv_blocked_cycles;
  uint64 write_request_arbiter_output_actv_blocked_by_arbitration_cycles;
  uint64 write_request_arbiter_output_actv_cycles_blocked_over_threshold;
  uint64 write_request_arbiter_status_block_write_request_cycles;
  uint64 write_request_arbiter_status_block_write_blocked_cycles;
  uint64 write_request_arbiter_status_block_write_blocked_by_arbitration_cycles;
  uint64 write_request_arbiter_status_block_write_cycles_blocked_over_threshold;
  uint64 address_translation_arbiter_instruction_request_cycles;
  uint64 address_translation_arbiter_instruction_blocked_cycles;
  uint64 address_translation_arbiter_instruction_blocked_by_arbitration_cycles;
  uint64 address_translation_arbiter_instruction_cycles_blocked_over_threshold;
  uint64 address_translation_arbiter_input_actv_request_cycles;
  uint64 address_translation_arbiter_input_actv_blocked_cycles;
  uint64 address_translation_arbiter_input_actv_blocked_by_arbitration_cycles;
  uint64 address_translation_arbiter_input_actv_cycles_blocked_over_threshold;
  uint64 address_translation_arbiter_param_request_cycles;
  uint64 address_translation_arbiter_param_blocked_cycles;
  uint64 address_translation_arbiter_param_blocked_by_arbitration_cycles;
  uint64 address_translation_arbiter_param_cycles_blocked_over_threshold;
  uint64 address_translation_arbiter_status_block_write_request_cycles;
  uint64 address_translation_arbiter_status_block_write_blocked_cycles;
  uint64
      address_translation_arbiter_status_block_write_blocked_by_arbitration_cycles;  // NOLINT
  uint64
      address_translation_arbiter_status_block_write_cycles_blocked_over_threshold;  // NOLINT
  uint64 address_translation_arbiter_output_actv_request_cycles;
  uint64 address_translation_arbiter_output_actv_blocked_cycles;
  uint64 address_translation_arbiter_output_actv_blocked_by_arbitration_cycles;
  uint64 address_translation_arbiter_output_actv_cycles_blocked_over_threshold;
  uint64 address_translation_arbiter_queue_fetch_request_cycles;
  uint64 address_translation_arbiter_queue_fetch_blocked_cycles;
  uint64 address_translation_arbiter_queue_fetch_blocked_by_arbitration_cycles;
  uint64 address_translation_arbiter_queue_fetch_cycles_blocked_over_threshold;
  uint64 issued_interrupt_count;
  uint64 data_read_16byte_count;
  uint64 waiting_for_tag_cycles;
  uint64 waiting_for_axi_cycles;
  uint64 simple_translations;
  uint64 instruction_credits_per_cycle_sum;
  uint64 input_actv_credits_per_cycle_sum;
  uint64 param_credits_per_cycle_sum;
  uint64 output_actv_credits_per_cycle_sum;
  uint64 status_block_write_credits_per_cycle_sum;
  uint64 queue_fetch_credits_per_cycle_sum;
  uint64 page_table_request_credits_per_cycle_sum;
  uint64 output_actv_queue_control;
  uint64 output_actv_queue_status;
  uint64 output_actv_queue_descriptor_size;
  uint64 output_actv_queue_minimum_size;
  uint64 output_actv_queue_maximum_size;
  uint64 output_actv_queue_base;
  uint64 output_actv_queue_status_block_base;
  uint64 output_actv_queue_size;
  uint64 output_actv_queue_tail;
  uint64 output_actv_queue_fetched_head;
  uint64 output_actv_queue_completed_head;
  uint64 output_actv_queue_int_control;
  uint64 output_actv_queue_int_status;
  uint64 instruction_queue_control;
  uint64 instruction_queue_status;
  uint64 instruction_queue_descriptor_size;
  uint64 instruction_queue_minimum_size;
  uint64 instruction_queue_maximum_size;
  uint64 instruction_queue_base;
  uint64 instruction_queue_status_block_base;
  uint64 instruction_queue_size;
  uint64 instruction_queue_tail;
  uint64 instruction_queue_fetched_head;
  uint64 instruction_queue_completed_head;
  uint64 instruction_queue_int_control;
  uint64 instruction_queue_int_status;
  uint64 input_actv_queue_control;
  uint64 input_actv_queue_status;
  uint64 input_actv_queue_descriptor_size;
  uint64 input_actv_queue_minimum_size;
  uint64 input_actv_queue_maximum_size;
  uint64 input_actv_queue_base;
  uint64 input_actv_queue_status_block_base;
  uint64 input_actv_queue_size;
  uint64 input_actv_queue_tail;
  uint64 input_actv_queue_fetched_head;
  uint64 input_actv_queue_completed_head;
  uint64 input_actv_queue_int_control;
  uint64 input_actv_queue_int_status;
  uint64 param_queue_control;
  uint64 param_queue_status;
  uint64 param_queue_descriptor_size;
  uint64 param_queue_minimum_size;
  uint64 param_queue_maximum_size;
  uint64 param_queue_base;
  uint64 param_queue_status_block_base;
  uint64 param_queue_size;
  uint64 param_queue_tail;
  uint64 param_queue_fetched_head;
  uint64 param_queue_completed_head;
  uint64 param_queue_int_control;
  uint64 param_queue_int_status;
  uint64 sc_host_int_control;
  uint64 sc_host_int_status;
  uint64 top_level_int_control;
  uint64 top_level_int_status;
  uint64 fatal_err_int_control;
  uint64 fatal_err_int_status;
  uint64 sc_host_int_count;
  uint64 dma_pause;
  uint64 dma_paused;
  uint64 status_block_update;
  uint64 hib_error_status;
  uint64 hib_error_mask;
  uint64 hib_first_error_status;
  uint64 hib_first_error_timestamp;
  uint64 hib_inject_error;
  uint64 read_request_arbiter;
  uint64 write_request_arbiter;
  uint64 address_translation_arbiter;
  uint64 sender_queue_threshold;
  uint64 page_fault_address;
  uint64 instruction_credits;
  uint64 input_actv_credits;
  uint64 param_credits;
  uint64 output_actv_credits;
  uint64 pause_state;
  uint64 snapshot;
  uint64 idle_assert;
  uint64 wire_int_pending_bit_array;
  uint64 tileconfig0;
  uint64 tileconfig1;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // DARWINN_DRIVER_CONFIG_DEBUG_HIB_USER_CSR_OFFSETS_H_
