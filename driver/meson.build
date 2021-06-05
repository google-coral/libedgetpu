HEADERS = [
  'hardware_structures.h',
  'device_buffer.h',
  'bitfield.h',
  'aligned_allocator.h',
  'allocator.h',
  'dma_info.h',
  'executable_util.h',
  'default_telemeter.h',
  'device_buffer_mapper.h',
  'instruction_buffers.h',
  'package_verifier.h',
  'package_registry.h',
  'driver_helper.h',
  'driver_factory.h',
  'mmio_driver.h',
  'run_controller.h',
  'scalar_core_controller.h',
  'driver.h',
  'single_tpu_request.h',
  'request.h',
  'top_level_handler.h',
  'dma_chunker.h',
  'real_time_dma_scheduler.h',
  'single_queue_dma_scheduler.h',
  'dma_scheduler.h',
  'tpu_request.h',
  'dma_info_extractor.h',
]

install_headers(HEADERS, subdir : 'driver')

driver_hardware_structures = declare_dependency(
  sources : 'hardware_structures.h',
  dependencies : [
    port_integral_types,
    port_macros,
  ],
)

driver_device_buffer = declare_dependency(
  sources : [
    'device_buffer.cc',
    'device_buffer.h',
  ],
  dependencies : [
    port,
  ],
  include_directories : include_directories('..')
)

driver_util = declare_dependency(
  sources : 'bitfield.h',
  dependencies : [
    port_integral_types,
    port_logging,
  ],
)

subdir('registers')
subdir('config')
subdir('memory')
subdir('interrupt')
subdir('mmio')
subdir('kernel')

driver_allocator = declare_dependency(
  sources : [
    'aligned_allocator.cc',
    'aligned_allocator.h',
    'allocator.cc',
    'allocator.h',
  ],
  dependencies : [
    api_allocated_buffer,
    api_buffer,
    port,
  ],
  include_directories : include_directories('..'),
)

driver_dma_info = declare_dependency(
  sources : [
    'dma_info.cc',
    'dma_info.h',
  ],
  dependencies : [
    driver_device_buffer,
    port,
  ],
  include_directories : include_directories('..'),
)

driver_executable_util = declare_dependency(
  sources : [
    'executable_util.cc',
    'executable_util.h',
  ],
  dependencies : [
    api_buffer,
    executable_fbs,
    port,
  ],
  include_directories : include_directories('..')
)

driver_default_telemeter = declare_dependency(
  sources : 'default_telemeter.h',
  dependencies : [
    api_telemeter_interface,
  ]
)

driver_device_buffer_mapper = declare_dependency(
  sources : [
    'device_buffer_mapper.cc',
    'device_buffer_mapper.h',
  ],
  dependencies : [
    driver_device_buffer,
    driver_hardware_structures,
    api_buffer,
    driver_memory_address_space,
    driver_memory_address_utilities,
    driver_memory_dma_direction,
    port,
    port_tracing,
  ],
  include_directories : include_directories('..')
)

driver_instruction_buffers = declare_dependency(
  sources : [
    'instruction_buffers.cc',
    'instruction_buffers.h',
  ],
  dependencies : [
    driver_allocator,
    driver_device_buffer_mapper,
    driver_executable_util,
    api_buffer,
    executable_fbs,
    port,
    port_tracing,
  ],
  include_directories : include_directories('..')
)

driver_package_verifier = declare_dependency(
  sources : [
    'package_verifier.cc',
    'package_verifier.h',
  ],
  dependencies : [
    executable_fbs,
    port,
  ],
  include_directories : include_directories('..')
)

driver_package_registry = declare_dependency(
  sources : [
    'package_registry.cc',
    'package_registry.h',
  ],
  dependencies : [
    driver_allocator,
    driver_device_buffer_mapper,
    driver_device_buffer,
    driver_instruction_buffers,
    driver_package_verifier,
    api_tensor_util,
    api_buffer,
    api_chip,
    api_driver_options_fbs,
    api_execution_context_interface,
    api_layer_information,
    api_package_reference,
    api_runtime_version,
    driver_memory_dram_allocator,
    executable_fbs,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_dma_info_extractor = declare_dependency(
  sources : [
    'dma_info_extractor.cc',
    'dma_info_extractor.h',
  ],
  dependencies : [
    driver_device_buffer,
    driver_dma_info,
    driver_package_registry,
    driver_memory_address_utilities,
    executable_fbs,
    port,
  ],
  include_directories : include_directories('..')
)

driver_tpu_request = declare_dependency(
  sources : 'tpu_request.h',
  dependencies : [
    driver_dma_info,
    driver_package_registry,
    api_buffer,
    api_request,
    port,
  ],
)

driver_dma_scheduler = declare_dependency(
  sources : 'dma_scheduler.h',
  dependencies : [
    driver_dma_info,
    driver_tpu_request,
    api_driver,
    port,
  ],
)

driver_single_queue_dma_scheduler = declare_dependency(
  sources : [
    'single_queue_dma_scheduler.cc',
    'single_queue_dma_scheduler.h',
  ],
  dependencies : [
    driver_dma_info,
    driver_dma_scheduler,
    driver_tpu_request,
    api_driver,
    api_watchdog,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
    executable_fbs,
  ],
  include_directories : include_directories('..')
)

driver_real_time_dma_scheduler = declare_dependency(
  sources : [
    'real_time_dma_scheduler.cc',
    'real_time_dma_scheduler.h',
  ],
  dependencies : [
    driver_dma_info,
    driver_dma_scheduler,
    driver_package_registry,
    driver_single_queue_dma_scheduler,
    driver_tpu_request,
    absl_strings,
    api_driver,
    api_package_reference,
    api_timing,
    api_watchdog,
    executable_fbs,
    driver_shared_time_stamper,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
  include_directories : include_directories('..')
)

driver_dma_chunker = declare_dependency(
  sources : [
    'dma_chunker.cc',
    'dma_chunker.h',
  ],
  dependencies : [
    driver_device_buffer,
    port,
  ],
  include_directories : include_directories('..')
)

driver_top_level_handler = declare_dependency(
  sources : 'top_level_handler.h',
  dependencies : [
    port,
  ],
)

driver_request = declare_dependency(
  sources : [
    'request.cc',
    'request.h',
  ],
  dependencies : [
    driver_tpu_request,
    api_request,
    driver_shared_time_stamper,
    driver_shared_time_stamper_driver_time_stamper_factory,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
    executable_fbs,
  ],
  include_directories : include_directories('..')
)

driver_single_tpu_request = declare_dependency(
  sources : [
    'single_tpu_request.cc',
    'single_tpu_request.h',
  ],
  dependencies : [
    driver_allocator,
    driver_device_buffer,
    driver_device_buffer_mapper,
    driver_dma_info,
    driver_dma_info_extractor,
    driver_executable_util,
    driver_hardware_structures,
    driver_instruction_buffers,
    driver_package_registry,
    driver_request,
    driver_tpu_request,
    api_allocated_buffer,
    api_buffer,
    driver_memory_address_space,
    driver_memory_dram_allocator,
    executable_fbs,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_driver = declare_dependency(
  sources : [
    'driver.cc',
    'driver.h',
  ],
  dependencies : [
    driver_default_telemeter,
    driver_device_buffer_mapper,
    driver_package_registry,
    driver_request,
    driver_tpu_request,
    absl_strings,
    api_buffer,
    api_chip,
    api_execution_context_interface,
    api_package_reference,
    api_request,
    api_telemeter_interface,
    driver_memory_dma_direction,
    driver_shared_time_stamper,
    executable_fbs,
    port,
    port_blocking_counter,
    port_shared_mutex,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
    threads,
  ],
)

driver_run_controller = declare_dependency(
  sources : [
    'run_controller.cc',
    'run_controller.h',
  ],
  dependencies : [
    driver_hardware_structures,
    driver_config,
    driver_config_register_constants,
    driver_registers,
    port,
  ],
  include_directories : include_directories('..')
)

driver_scalar_core_controller = declare_dependency(
  sources : [
    'scalar_core_controller.cc',
    'scalar_core_controller.h',
  ],
  dependencies : [
    driver_config,
    driver_interrupt_interrupt_controller,
    driver_registers,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_mmio_driver = declare_dependency(
  sources : [
    'mmio_driver.cc',
    'mmio_driver.h',
  ],
  dependencies : [
    driver_allocator,
    driver_device_buffer,
    driver_device_buffer_mapper,
    driver_dma_info_extractor,
    driver_driver,
    driver_hardware_structures,
    driver_package_registry,
    driver_real_time_dma_scheduler,
    driver_run_controller,
    driver_scalar_core_controller,
    driver_single_tpu_request,
    driver_top_level_handler,
    driver_tpu_request,
    api_allocated_buffer,
    api_buffer,
    api_watchdog,
    driver_config,
    driver_config_register_constants,
    driver_interrupt_interrupt_controller_interface,
    driver_interrupt_interrupt_handler,
    driver_interrupt_top_level_interrupt_manager,
    driver_memory_address_space,
    driver_memory_address_utilities,
    driver_memory_dma_direction,
    driver_memory_dram_allocator,
    driver_memory_mmu_mapper,
    driver_mmio_host_queue,
    driver_registers,
    driver_shared_time_stamper,
    driver_shared_time_stamper_driver_time_stamper,
    executable_fbs,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_factory_srcs = ['driver_factory.cc']

if is_windows
  driver_factory_srcs += ['driver_factory_windows.cc']
elif is_darwin
  driver_factory_srcs += ['driver_factory_darwin.cc']
else
  driver_factory_srcs += ['driver_factory_default.cc']
endif

driver_driver_factory = declare_dependency(
  sources : driver_factory_srcs + [
    'driver_factory.h',
  ],
  dependencies : [
    driver_driver,
    api_chip,
    api_driver_factory,
    api_driver_options_fbs,
    api_driver_options_helper,
    driver_config,
    driver_kernel_gasket_ioctl,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
    executable_fbs,
  ],
  include_directories : include_directories('..')
)

driver_driver_helper = declare_dependency(
  sources : [
    'driver_helper.cc',
    'driver_helper.h',
  ],
  dependencies : [
    driver_executable_util,
    driver_package_registry,
    api_buffer,
    api_chip,
    api_driver,
    api_package_reference,
    api_request,
    api_telemeter_interface,
    api_timing,
    executable_fbs,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_libdarwinn_driver_lds = files('libdarwinn_driver.lds')[0]

subdir('usb')
subdir('beagle')
