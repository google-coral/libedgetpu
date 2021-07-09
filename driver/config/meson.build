driver_config_headers = [
  'apex_csr_offsets.h',
  'beagle_csr_helper.h',
  'breakpoint_csr_offsets.h',
  'cb_bridge_csr_offsets.h',
  'chip_config.h',
  'chip_structures.h',
  'common_csr_helper.h',
  'debug_hib_user_csr_offsets.h',
  'debug_scalar_core_csr_offsets.h',
  'debug_tile_csr_offsets.h',
  'hib_kernel_csr_offsets.h',
  'hib_user_csr_offsets.h',
  'interrupt_csr_offsets.h',
  'lpm_csr_offsets.h',
  'memory_csr_offsets.h',
  'misc_csr_offsets.h',
  'msix_csr_offsets.h',
  'power_throttle_csr_helper.h',
  'queue_csr_offsets.h',
  'register_file_csr_offsets.h',
  'scalar_core_csr_offsets.h',
  'scu_csr_offsets.h',
  'sync_flag_csr_offsets.h',
  'tile_config_csr_offsets.h',
  'tile_csr_offsets.h',
  'tile_thread_csr_offsets.h',
  'tile_thread_trace_csr_offsets.h',
  'trace_csr_offsets.h',
  'usb_csr_offsets.h',
  'wire_csr_offsets.h',
]

HEADERS = driver_config_headers + ['register_constants.h']

install_headers(HEADERS, subdir : 'driver/config')

driver_config = declare_dependency(
  sources : driver_config_headers,
  dependencies : [
    api_chip,
    driver_util,
    port_integral_types,
    port_logging,
    port_unreachable,
  ],
)

driver_config_register_constants = declare_dependency(
  sources : 'register_constants.h',
  dependencies : [
    port_integral_types,
  ]
)

subdir('beagle')
