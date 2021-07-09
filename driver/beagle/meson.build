headers = [
  'beagle_pci_driver_provider.h',
  'beagle_top_level_interrupt_manager.h',
  'beagle_top_level_handler.h',
  'beagle_kernel_top_level_handler.h',
]

install_headers(headers, subdir : 'driver/beagle')

driver_beagle_beagle_top_level_interrupt_manager = declare_dependency(
  sources : [
    'beagle_top_level_interrupt_manager.cc',
    'beagle_top_level_interrupt_manager.h',
  ],
  dependencies : [
    driver_config,
    driver_interrupt_interrupt_controller_interface,
    driver_interrupt_top_level_interrupt_manager,
    driver_registers,
    port,
    port_fileio,
  ],
  include_directories : include_directories('../..'),
)

driver_beagle_beagle_top_level_handler = declare_dependency(
  sources : [
    'beagle_top_level_handler.cc',
    'beagle_top_level_handler.h',
  ],
  dependencies : [
    api_driver_options_fbs,
    driver_top_level_handler,
    driver_config,
    driver_registers,
    port,
    port_fileio,
  ],
  include_directories : include_directories('../..'),
)

driver_beagle_beagle_ioctl = declare_dependency(
  sources : 'beagle_ioctl.h',
  dependencies : [
    driver_kernel_gasket_ioctl,
  ],
)

driver_beagle_beagle_kernel_top_level_handler = declare_dependency(
  sources : [
    'beagle_kernel_top_level_handler.cc',
    'beagle_kernel_top_level_handler.h',
  ],
  dependencies : [
    driver_beagle_beagle_ioctl,
    api_driver_options_fbs,
    driver_top_level_handler,
    port,
    port_fileio,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
  include_directories : include_directories('../..'),
)

if usb_enabled
  BEAGLE_USB_DRIVER_PROVIDER_DEPS = [
    driver_beagle_beagle_top_level_handler,
    driver_beagle_beagle_top_level_interrupt_manager,
    absl_strings,
    api_chip,
    api_driver,
    api_driver_options_fbs,
    driver_allocator,
    driver_driver_factory,
    driver_package_registry, driver_package_verifier,
    driver_run_controller,
    driver_scalar_core_controller,
    driver_config_beagle_beagle_chip_config,
    driver_interrupt_grouped_interrupt_controller,
    driver_interrupt_interrupt_controller,
    driver_interrupt_interrupt_controller_interface,
    driver_memory_null_dram_allocator,
    driver_shared_time_stamper_driver_time_stamper,
    driver_usb_usb_device_interface,
    driver_usb_usb_driver,
    driver_usb_usb_ml_commands,
    driver_usb_usb_registers,
    port,
    port_tracing,
  ]

  driver_beagle_beagle_usb_driver_provider = declare_dependency(
    sources : 'beagle_usb_driver_provider.cc',
    dependencies : BEAGLE_USB_DRIVER_PROVIDER_DEPS + [
      driver_usb_local_usb_device,
    ],
    include_directories : include_directories('../..'),
  )
endif

if pci_enabled
  BEAGLE_PCI_DRIVER_DEPS = [
    driver_beagle_beagle_kernel_top_level_handler,
    absl_strings,
    api_chip,
    api_driver,
    driver_allocator,
    driver_driver_factory,
    driver_hardware_structures,
    driver_mmio_driver,
    driver_package_registry,
    driver_package_verifier,
    driver_run_controller,
    driver_scalar_core_controller,
    driver_config,
    driver_config_beagle_beagle_chip_config,
    driver_interrupt_dummy_interrupt_controller,
    driver_interrupt_grouped_interrupt_controller,
    driver_interrupt_interrupt_handler,
    driver_interrupt_top_level_interrupt_manager,
    driver_kernel_kernel_coherent_allocator,
    driver_kernel_kernel_event_handler,
    driver_kernel_kernel_interrupt_handler,
    driver_kernel_kernel_mmu_mapper,
    driver_kernel_kernel_registers,
    driver_kernel_kernel_wire_interrupt_handler,
    driver_memory_dual_address_space,
    driver_memory_null_dram_allocator,
    driver_mmio_host_queue,
    driver_shared_time_stamper_driver_time_stamper,
    executable_fbs,
    port,
    port_fileio,
  ]

  driver_beagle_beagle_pci_driver_provider = declare_dependency(
    sources : 'beagle_pci_driver_provider.cc',
    dependencies : BEAGLE_PCI_DRIVER_DEPS,
    include_directories : include_directories('../..'),
  )
  if is_linux
    driver_beagle_beagle_pci_driver_provider_linux = declare_dependency(
      sources : 'beagle_pci_driver_provider_linux.cc',
      dependencies : BEAGLE_PCI_DRIVER_DEPS + [
        driver_beagle_beagle_pci_driver_provider,
        driver_kernel_linux_kernel_coherent_allocator_linux,
        driver_kernel_linux_kernel_event_handler_linux,
        driver_kernel_linux_kernel_registers_linux,
      ],
      include_directories : include_directories('../..')
    )
  endif

  if is_windows
    driver_beagle_beagle_pci_driver_provider_windows = declare_dependency(
      sources : 'beagle_pci_driver_provider_windows.cc',
      dependencies : BEAGLE_PCI_DRIVER_DEPS + [
        driver_beagle_beagle_pci_driver_provider,
        driver_kernel_windows_kernel_coherent_allocator_windows,
        driver_kernel_windows_kernel_event_handler_windows,
        driver_kernel_windows_kernel_registers_windows,
      ],
      include_directories : include_directories('../..'),
    )
  endif
endif

if usb_enabled and pci_enabled
  if is_linux
    driver_beagle_beagle_all_driver_provider_linux = declare_dependency(
      sources : [
        'beagle_pci_driver_provider.cc',
        'beagle_pci_driver_provider_linux.cc',
        'beagle_usb_driver_provider.cc',
        'beagle_pci_driver_provider.h',
      ],
      dependencies : BEAGLE_PCI_DRIVER_DEPS + [
        driver_beagle_beagle_top_level_handler,
        driver_beagle_beagle_top_level_interrupt_manager,
        api_driver_options_fbs,
        driver_interrupt_interrupt_controller,
        driver_interrupt_interrupt_controller_interface,
        driver_kernel_linux_kernel_coherent_allocator_linux,
        driver_kernel_linux_kernel_event_handler_linux,
        driver_kernel_linux_kernel_registers_linux,
        driver_usb_local_usb_device,
        driver_usb_usb_device_interface,
        driver_usb_usb_driver,
        driver_usb_usb_ml_commands,
        driver_usb_usb_registers,
        port_tracing,
      ],
      include_directories : include_directories('../..'),
    )
  endif

  if is_windows
    driver_beagle_beagle_all_driver_provider_windows = declare_dependency(
      sources : [
        'beagle_pci_driver_provider.cc',
        'beagle_pci_driver_provider_windows.cc',
        'beagle_usb_driver_provider.cc',
        'beagle_pci_driver_provider.h',
      ],
      dependencies : BEAGLE_PCI_DRIVER_DEPS + [
        driver_beagle_beagle_top_level_handler,
        driver_beagle_beagle_top_level_interrupt_manager,
        api_driver_options_fbs,
        driver_interrupt_interrupt_controller,
        driver_interrupt_interrupt_controller_interface,
        driver_kernel_windows_kernel_coherent_allocator_windows,
        driver_kernel_windows_kernel_event_handler_windows,
        driver_kernel_windows_kernel_registers_windows,
        driver_usb_local_usb_device,
        driver_usb_usb_device_interface,
        driver_usb_usb_driver,
        driver_usb_usb_ml_commands,
        driver_usb_usb_registers,
        port_tracing,
      ],
      include_directories : include_directories('../..'),
    )
  endif
endif

if usb_enabled
  libbeagle_usb = library(
    'beagle-usb',
    dependencies : [
      driver_beagle_beagle_usb_driver_provider,
    ],
    implicit_include_directories : false,
    include_directories : include_directories('../..'),
    link_args : [
      '-Wl,--version-script=../@0@'.format(driver_libdarwinn_driver_lds),
    ],
    link_depends : driver_libdarwinn_driver_lds,
    install : true,
  )
  pkgconfig.generate(libbeagle_usb)
endif

if pci_enabled
  libbeagle_pci = library(
    'beagle-pci',
    dependencies : [
      driver_beagle_beagle_pci_driver_provider,
    ],
    implicit_include_directories : false,
    include_directories : include_directories('../..'),
    link_args : [
      '-Wl,--version-script=../@0@'.format(driver_libdarwinn_driver_lds),
    ],
    link_depends : driver_libdarwinn_driver_lds,
    install : true,
  )
  pkgconfig.generate(libbeagle_pci)
endif
