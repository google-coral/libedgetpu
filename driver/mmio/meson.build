HEADERS = [
  'coherent_allocator.h',
  'host_queue.h',
]

install_headers(HEADERS, subdir : 'driver/mmio')

driver_mmio_coherent_allocator = declare_dependency(
  sources : [
    'coherent_allocator.cc',
    'coherent_allocator.h',
  ],
  dependencies : [
    api_buffer,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_mmio_host_queue = declare_dependency(
  sources : 'host_queue.h',
  dependencies : [
    driver_mmio_coherent_allocator,
    api_buffer,
    driver_device_buffer,
    driver_hardware_structures,
    driver_config,
    driver_memory_address_space,
    driver_memory_dma_direction,
    driver_registers,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)
