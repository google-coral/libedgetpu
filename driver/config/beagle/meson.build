headers = [
  'beagle_chip_config.h',
  'beagle_csr_offsets.h',
  'beagle_chip_structures.h'
]

install_headers(headers, subdir : 'driver/config/beagle')

driver_config_beagle_beagle_config = declare_dependency(
  sources : [
    'beagle_chip_structures.h',
    'beagle_csr_offsets.h',
  ],
  dependencies : [
    driver_config,
    driver_config_register_constants,
  ],
)

driver_config_beagle_beagle_chip_config = declare_dependency(
  sources : [
    'beagle_chip_config.h',
  ],
  dependencies : [
    driver_config_beagle_beagle_config,
    driver_config,
    port_logging,
    port_unreachable,
  ],
)
