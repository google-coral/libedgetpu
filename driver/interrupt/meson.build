HEADERS = [
  'interrupt_handler.h',
  'wire_interrupt_handler.h',
  'interrupt_controller_interface.h',
  'dummy_interrupt_controller.h',
  'interrupt_controller.h',
  'grouped_interrupt_controller.h',
  'top_level_interrupt_manager.h',
]

install_headers(HEADERS, subdir : 'driver/interrupt')

driver_interrupt_interrupt_handler = declare_dependency(
  sources : 'interrupt_handler.h',
  dependencies : [
    port,
  ],
)

driver_interrupt_wire_interrupt_handler = declare_dependency(
  sources : [
    'wire_interrupt_handler.cc',
    'wire_interrupt_handler.h',
  ],
  dependencies : [
    driver_interrupt_interrupt_handler,
    driver_config,
    driver_registers,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_tracing,
    threads,
  ],
)

driver_interrupt_interrupt_controller_interface = declare_dependency(
  sources : 'interrupt_controller_interface.h',
  dependencies : [
    driver_config_register_constants,
    port,
  ],
)

driver_interrupt_dummy_interrupt_controller = declare_dependency(
  sources : 'dummy_interrupt_controller.h',
  dependencies : [
    driver_config_register_constants,
    port,
  ],
)

driver_interrupt_interrupt_controller = declare_dependency(
  sources : [
    'interrupt_controller.cc',
    'interrupt_controller.h',
  ],
  dependencies : [
    driver_interrupt_interrupt_controller_interface,
    driver_config,
    driver_config_register_constants,
    driver_registers,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_interrupt_grouped_interrupt_controller = declare_dependency(
  sources : [
    'grouped_interrupt_controller.cc',
    'grouped_interrupt_controller.h',
  ],
  dependencies : [
    driver_interrupt_interrupt_controller,
    driver_interrupt_interrupt_controller_interface,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_interrupt_top_level_interrupt_manager = declare_dependency(
  sources : [
    'top_level_interrupt_manager.cc',
    'top_level_interrupt_manager.h',
  ],
  dependencies : [
    driver_interrupt_interrupt_controller_interface,
    port,
  ],
  include_directories : include_directories('../..'),
)
