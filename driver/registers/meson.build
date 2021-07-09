HEADERS = [
  'registers.h',
  'socket_registers.h',
]

install_headers(HEADERS, subdir : 'driver/registers')

driver_registers = declare_dependency(
  sources : [
    'registers.cc',
    'registers.h',
  ],
  dependencies : [
    driver_shared_registers,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_socket_registers = declare_dependency(
  sources : [
    'socket_registers.cc',
    'socket_registers.h',
  ],
  dependencies : [
    driver_registers,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
  ],
)
