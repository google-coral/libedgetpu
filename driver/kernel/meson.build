headers = [
  'common_gasket_ioctl.inc',
  'gasket_ioctl.h',
  'kernel_coherent_allocator.h',
  'kernel_event.h',
  'kernel_event_handler.h',
  'kernel_interrupt_handler.h',
  'kernel_mmu_mapper.h',
  'kernel_registers.h',
  'kernel_wire_interrupt_handler.h',
]
install_headers(headers, subdir : 'driver/kernel')

if is_windows
  windows_headers = [
    'windows/kernel_event_windows.h',
    'windows/kernel_event_handler_windows.h',
    'windows/kernel_coherent_allocator_windows.h',
    'windows/kernel_registers_windows.h',
    'windows/windows_gasket_ioctl.inc'
  ]
  install_headers(windows_headers, subdir : 'driver/kernel/windows')
endif

if is_linux
  linux_headers = [
    'linux/kernel_event_linux.h',
    'linux/kernel_event_handler_linux.h',
    'linux/kernel_coherent_allocator_linux.h',
    'linux/kernel_registers_linux.h',
  ]
  install_headers(linux_headers, subdir : 'driver/kernel/linux')
endif

driver_kernel_gasket_ioctl_deps = []

if is_windows
  copy = find_program('copy')
else
  copy = find_program('cp')
endif

if is_windows
  driver_kernel_windows_gasket_ioctl_windows = declare_dependency(
    sources : custom_target(
      'windows_gasket_ioctl_inc',
      input : 'windows/windows_gasket_ioctl.inc',
      output : 'windows_gasket_ioctl.inc',
      command : [copy, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : 'include/driver/kernel/windows'
    ),
    dependencies : [
      port,
    ],
  )

  driver_kernel_gasket_ioctl_deps += [
    driver_kernel_windows_gasket_ioctl_windows,
  ]
endif

driver_kernel_gasket_ioctl = declare_dependency(
  sources : [
    'gasket_ioctl.h',
    custom_target(
      'common_gasket_ioctl_inc',
      input : 'common_gasket_ioctl.inc',
      output : 'common_gasket_ioctl.inc',
      command : [copy, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : 'include/driver/kernel'
    ),
  ],
  dependencies : driver_kernel_gasket_ioctl_deps,
)

driver_kernel_kernel_event = declare_dependency(
  sources : 'kernel_event.h',
  dependencies : [
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_kernel_kernel_event_handler = declare_dependency(
  sources : [
    'kernel_event_handler.cc',
    'kernel_event_handler.h',
  ],
  dependencies : [
    driver_kernel_gasket_ioctl,
    driver_kernel_kernel_event,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

driver_kernel_kernel_interrupt_handler = declare_dependency(
  sources : [
    'kernel_interrupt_handler.cc',
    'kernel_interrupt_handler.h',
  ],
  dependencies : [
    driver_config,
    driver_interrupt_interrupt_handler,
    driver_interrupt_wire_interrupt_handler,
    driver_kernel_kernel_event_handler,
    driver_registers,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_kernel_kernel_wire_interrupt_handler = declare_dependency(
  sources : [
    'kernel_wire_interrupt_handler.cc',
    'kernel_wire_interrupt_handler.h',
  ],
  dependencies : [
    driver_interrupt_interrupt_handler,
    driver_interrupt_wire_interrupt_handler,
    driver_kernel_kernel_event_handler,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_kernel_kernel_coherent_allocator = declare_dependency(
  sources : [
    'kernel_coherent_allocator.cc',
    'kernel_coherent_allocator.h',
  ],
  dependencies : [
    driver_kernel_gasket_ioctl,
    driver_hardware_structures,
    driver_mmio_coherent_allocator,
    port,
    port_fileio,
  ],
  include_directories : include_directories('../..'),
)

driver_kernel_kernel_registers = declare_dependency(
  sources : [
    'kernel_registers.cc',
    'kernel_registers.h',
  ],
  dependencies : [
    driver_kernel_gasket_ioctl,
    absl_strings,
    driver_registers,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)

driver_kernel_kernel_mmu_mapper = declare_dependency(
  sources : [
    'kernel_mmu_mapper.cc',
    'kernel_mmu_mapper.h',
  ],
  dependencies : [
    driver_kernel_gasket_ioctl,
    driver_hardware_structures,
    driver_memory_dma_direction,
    driver_memory_mmu_mapper,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
  ],
)

if is_linux
  driver_kernel_linux_kernel_event_linux = declare_dependency(
    sources : [
      'linux/kernel_event_linux.cc',
      'linux/kernel_event_linux.h',
    ],
    dependencies : [
      driver_kernel_kernel_event,
      port,
      port_fileio,
      port_std_mutex_lock,
      port_thread_annotations,
      port_tracing,
      threads,
    ],
  )

  driver_kernel_linux_kernel_event_handler_linux = declare_dependency(
    sources : [
      'linux/kernel_event_handler_linux.cc',
      'linux/kernel_event_handler_linux.h',
    ],
    dependencies : [
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_event,
      driver_kernel_kernel_event_handler,
      driver_kernel_linux_kernel_event_linux,
      port,
      port_fileio,
      port_std_mutex_lock,
      port_thread_annotations,
      port_tracing,
    ],
  )

  driver_kernel_linux_kernel_coherent_allocator_linux = declare_dependency(
    sources : [
      'linux/kernel_coherent_allocator_linux.cc',
      'linux/kernel_coherent_allocator_linux.h',
    ],
    dependencies : [
      driver_hardware_structures,
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_coherent_allocator,
      driver_mmio_coherent_allocator,
      port,
      port_fileio,
    ],
    include_directories : include_directories('../..'),
  )

  driver_kernel_linux_kernel_registers_linux = declare_dependency(
    sources: [
      'linux/kernel_registers_linux.cc',
      'linux/kernel_registers_linux.h',
    ],
    dependencies : [
      absl_strings,
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_registers,
      driver_registers,
      port,
      port_fileio,
      port_std_mutex_lock,
      port_thread_annotations,
    ],
  )
endif

if is_windows
  driver_kernel_windows_kernel_event_windows = declare_dependency(
    sources : [
      'windows/kernel_event_windows.cc',
      'windows/kernel_event_windows.h',
    ],
    dependencies : [
      driver_kernel_windows_gasket_ioctl_windows,
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_event,
      port,
      port_fileio,
      port_std_mutex_lock,
      port_thread_annotations,
      port_tracing,
    ],
  )

  driver_kernel_windows_kernel_event_handler_windows = declare_dependency(
    sources : [
      'windows/kernel_event_handler_windows.cc',
      'windows/kernel_event_handler_windows.h',
    ],
    dependencies : [
      driver_kernel_windows_gasket_ioctl_windows,
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_event,
      driver_kernel_kernel_event_handler,
      driver_kernel_windows_kernel_event_windows,
      port,
      port_fileio,
      port_std_mutex_lock,
      port_thread_annotations,
      port_tracing,
    ],
  )

  driver_kernel_windows_kernel_coherent_allocator_windows = declare_dependency(
    sources : [
      'windows/kernel_coherent_allocator_windows.cc',
      'windows/kernel_coherent_allocator_windows.h',
    ],
    dependencies : [
      driver_kernel_windows_gasket_ioctl_windows,
      driver_hardware_structures,
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_coherent_allocator,
      driver_mmio_coherent_allocator,
      port,
      port_fileio,
    ],
    include_directories : include_directories('../..'),
  )

  driver_kernel_windows_kernel_registers_windows = declare_dependency(
    sources : [
      'windows/kernel_registers_windows.cc',
      'windows/kernel_registers_windows.h',
    ],
    dependencies : [
      driver_kernel_windows_gasket_ioctl_windows,
      absl_strings,
      driver_kernel_gasket_ioctl,
      driver_kernel_kernel_registers,
      driver_registers,
      port,
      port_fileio,
      port_std_mutex_lock,
      port_thread_annotations,
    ],
  )
endif
