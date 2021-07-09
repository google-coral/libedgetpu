HEADERS = [
  'dma_direction.h',
  'address_utilities.h',
  'mmu_mapper.h',
  'address_space.h',
  'mmio_address_space.h',
  'address_space_allocator.h',
  'buddy_allocator.h',
  'buddy_address_space.h',
  'dram_allocator.h',
  'fake_mmu_mapper.h',
  'dual_address_space.h',
  'nop_address_space.h',
  'null_dram_allocator.h',
  'fake_dram_allocator.h',
]

install_headers(HEADERS, subdir : 'driver/memory')

driver_memory_dma_direction = declare_dependency(
  sources : 'dma_direction.h',
)

driver_memory_address_utilities = declare_dependency(
  sources : 'address_utilities.h',
  dependencies : [
    driver_hardware_structures,
    port,
  ],
)

driver_memory_mmu_mapper = declare_dependency(
  sources : [
    'mmu_mapper.cc',
    'mmu_mapper.h',
  ],
  dependencies : [
    driver_memory_address_utilities,
    api_buffer,
    driver_device_buffer,
    driver_memory_dma_direction,
    port,
    port_tracing,
  ],
  include_directories : include_directories('../..'),
)

driver_memory_address_space = declare_dependency(
  sources : 'address_space.h',
  dependencies : [
    driver_memory_dma_direction,
    api_buffer,
    driver_device_buffer,
    port,
  ],
)

driver_memory_mmio_address_space = declare_dependency(
  sources : [
    'mmio_address_space.cc',
    'mmio_address_space.h',
  ],
  dependencies : [
    driver_memory_address_space,
    driver_memory_address_utilities,
    driver_memory_dma_direction,
    driver_memory_mmu_mapper,
    api_buffer,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_memory_address_space_allocator = declare_dependency(
  sources : 'address_space_allocator.h',
  dependencies : [
    port,
  ],
)
driver_memory_buddy_allocator = declare_dependency(
  sources : [
    'buddy_allocator.cc',
    'buddy_allocator.h',
  ],
  dependencies : [
    driver_memory_address_space_allocator,
    driver_memory_address_utilities,
    absl_strings,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)


driver_memory_buddy_address_space = declare_dependency(
  sources : [
    'buddy_address_space.cc',
    'buddy_address_space.h',
  ],
  dependencies : [
    driver_memory_address_utilities,
    driver_memory_buddy_allocator,
    driver_memory_dma_direction,
    driver_memory_mmio_address_space,
    driver_memory_mmu_mapper,
    driver_device_buffer,
    api_buffer,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_memory_dram_allocator = declare_dependency(
  sources : 'dram_allocator.h',
  dependencies : [
    api_dram_buffer,
    port,
  ],
)

driver_memory_fake_mmu_mapper = declare_dependency(
  sources : [
    'fake_mmu_mapper.cc',
    'fake_mmu_mapper.h',
  ],
  dependencies : [
    driver_memory_address_utilities,
    driver_memory_dma_direction,
    driver_memory_mmu_mapper,
    driver_hardware_structures,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
  include_directories : include_directories('../..'),
)

driver_memory_dual_address_space = declare_dependency(
  sources : [
    'dual_address_space.cc',
    'dual_address_space.h',
  ],
  dependencies : [
    driver_memory_address_space,
    driver_memory_buddy_address_space,
    driver_memory_mmu_mapper,
    driver_hardware_structures,
    driver_config,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_memory_nop_address_space = declare_dependency(
  sources : [
    'nop_address_space.cc',
    'nop_address_space.h',
  ],
  dependencies : [
    driver_memory_address_space,
    driver_memory_dma_direction,
    api_buffer,
    driver_device_buffer,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_memory_null_dram_allocator = declare_dependency(
  sources : 'null_dram_allocator.h',
  dependencies : [
    driver_memory_dram_allocator,
    api_dram_buffer,
    port,
  ],
)

driver_memory_fake_dram_allocator = declare_dependency(
  sources : [
    'fake_dram_allocator.cc',
    'fake_dram_allocator.h',
  ],
  dependencies : [
    driver_memory_dram_allocator,
    api_dram_buffer,
    port,
  ],
  include_directories : include_directories('../..'),
)
