HEADERS = [
  'usb_device_interface.h',
  'libusb_options.h',
  'local_usb_device.h',
  'usb_standard_commands.h',
  'usb_dfu_commands.h',
  'usb_ml_commands.h',
  'usb_registers.h',
  'usb_dfu_util.h',
  'usb_io_request.h',
  'usb_driver.h',
]

install_headers(HEADERS, subdir : 'driver/usb')

driver_usb_usb_device_interface = declare_dependency(
  sources : 'usb_device_interface.h',
  dependencies : [
    port,
  ]
)

if is_windows
  libusb_options_srcs = ['libusb_options_windows.cc']
else
  libusb_options_srcs = ['libusb_options_default.cc']
endif

driver_usb_libusb_options = declare_dependency(
  sources : libusb_options_srcs,
  dependencies : [
    port,
    libusb,
  ],
  include_directories : include_directories('../..'),
)

driver_usb_local_usb_device = declare_dependency(
  sources : [
    'local_usb_device.cc',
    'local_usb_device.h',
  ],
  dependencies : [
    driver_usb_libusb_options,
    threads,
    driver_usb_usb_device_interface,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_usb_usb_standard_commands = declare_dependency(
  sources : [
    'usb_standard_commands.cc',
    'usb_standard_commands.h',
  ],
  dependencies : [
    driver_usb_usb_device_interface,
    port,
    port_std_mutex_lock,
  ],
)

driver_usb_usb_dfu_commands = declare_dependency(
  sources : [
    'usb_dfu_commands.cc',
    'usb_dfu_commands.h',
  ],
  dependencies : [
    driver_usb_usb_device_interface,
    driver_usb_usb_standard_commands,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_usb_usb_latest_firmware_header = declare_dependency(
  sources : custom_target(
    'driver_usb_usb_latest_firmware_header',
    input : [
      'apex_latest_single_ep.bin',
      'apex_latest_multi_ep.bin',
    ],
    output : 'usb_latest_firmware.h',
    command : [
      bash,
      '@CURRENT_SOURCE_DIR@/gen_usb_latest_firmware.sh',
      '@OUTPUT@',
      '@INPUT@',
    ],
    install : true,
    install_dir : 'include/driver/usb',
  )
)

driver_usb_usb_ml_commands = declare_dependency(
  sources : [
    'usb_ml_commands.cc',
    'usb_ml_commands.h',
  ],
  dependencies : [
    driver_usb_usb_device_interface,
    driver_usb_usb_standard_commands,
    port,
    port_std_mutex_lock,
  ],
)

driver_usb_usb_registers = declare_dependency(
  sources : [
    'usb_registers.cc',
    'usb_registers.h',
  ],
  dependencies : [
    driver_usb_usb_ml_commands,
    driver_registers,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_usb_usb_dfu_util = declare_dependency(
  sources : [
    'usb_dfu_util.cc',
    'usb_dfu_util.h',
  ],
  dependencies : [
    driver_usb_usb_device_interface,
    driver_usb_usb_dfu_commands,
    driver_usb_usb_standard_commands,
    port,
    port_tracing,
  ],
  include_directories : include_directories('../..'),
)

driver_usb_usb_io_request = declare_dependency(
  sources : [
    'usb_io_request.cc',
    'usb_io_request.h',
  ],
  dependencies : [
    driver_usb_usb_ml_commands,
    driver_device_buffer,
    driver_dma_chunker,
    driver_dma_info,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_usb_usb_driver = declare_dependency(
  sources : [
    'usb_driver.cc',
    'usb_driver.h',
  ],
  dependencies : [
    driver_usb_usb_latest_firmware_header,
    driver_usb_usb_device_interface,
    driver_usb_usb_dfu_commands,
    driver_usb_usb_dfu_util,
    driver_usb_usb_io_request,
    driver_usb_usb_ml_commands,
    driver_usb_usb_registers,
    api_buffer,
    api_watchdog,
    driver_driver,
    driver_allocator,
    driver_device_buffer,
    driver_device_buffer_mapper,
    driver_dma_chunker,
    driver_dma_info,
    driver_dma_info_extractor,
    driver_hardware_structures,
    driver_package_registry,
    driver_request,
    driver_run_controller,
    driver_single_queue_dma_scheduler,
    driver_single_tpu_request,
    driver_top_level_handler,
    driver_tpu_request,
    driver_config,
    driver_interrupt_interrupt_controller_interface,
    driver_interrupt_top_level_interrupt_manager,
    driver_memory_address_utilities,
    driver_memory_dma_direction,
    driver_memory_dram_allocator,
    driver_memory_nop_address_space,
    driver_registers,
    driver_shared_time_stamper,
    executable_fbs,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
  include_directories : include_directories('../..'),
)
