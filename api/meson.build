HEADERS = [
  'allocated_buffer.h',
  'buffer.h',
  'chip.h',
  'dram_buffer.h',
  'driver.h',
  'driver_factory.h',
  'driver_options_helper.h',
  'execution_context_interface.h',
  'layer_information.h',
  'package_reference.h',
  'request.h',
  'runtime_version.h',
  'telemeter_interface.h',
  'tensor_util.h',
  'timing.h',
  'watchdog.h',
]

install_headers(HEADERS, subdir : 'api')

api_chip = declare_dependency(
  sources : 'chip.h',
)

api_tensor_util = declare_dependency(
  sources : [
    'tensor_util.cc',
    'tensor_util.h',
  ],
  dependencies : [
    absl_optional,
    port,
    port_default_port,
    port_default_port_from_tf,
    port_string_util,
    executable_fbs,
  ],
  include_directories : include_directories('..'),
)

api_allocated_buffer = declare_dependency(
  sources : [
    'allocated_buffer.cc',
    'allocated_buffer.h',
  ],
  dependencies : [
    port,
    port_default_port_from_tf,
  ],
  include_directories : include_directories('..'),
)

api_dram_buffer = declare_dependency(
  sources : 'dram_buffer.h',
  dependencies : [
    port,
  ],
)

api_buffer = declare_dependency(
  sources : [
    'buffer.cc',
    'buffer.h',
  ],
  dependencies : [
    api_allocated_buffer,
    api_dram_buffer,
    port,
    port_default_port,
    port_default_port_from_tf,
  ],
  include_directories : include_directories('..'),
)

api_layer_information = declare_dependency(
  sources : [
    'layer_information.cc',
    'layer_information.h',
  ],
  dependencies : [
    api_buffer,
    api_tensor_util,
    absl_optional,
    port,
    port_default_port,
    port_default_port_from_tf,
    executable_fbs,
  ],
  include_directories : include_directories('..'),
)

api_request = declare_dependency(
  sources : 'request.h',
  dependencies : [
    api_buffer,
    port,
  ],
)

api_execution_context_interface = declare_dependency(
  sources : 'execution_context_interface.h',
)

api_package_reference = declare_dependency(
  sources : 'package_reference.h',
  dependencies : [
    api_execution_context_interface,
    api_layer_information,
    port,
    executable_fbs,
  ],
)

api_timing = declare_dependency(
  sources : 'timing.h',
  dependencies : [
    port,
  ],
)

api_telemeter_interface = declare_dependency(
  sources : 'telemeter_interface.h',
  dependencies : [
    api_execution_context_interface,
    port,
  ],
)

api_driver = declare_dependency(
  sources : 'driver.h',
  dependencies : [
    api_buffer,
    api_package_reference,
    api_request,
    api_timing,
    api_telemeter_interface,
    port,
  ],
)

api_driver_options_fbs = declare_dependency(
  sources : custom_target(
    'api_driver_options_fbs',
    output : '.',
    input : 'driver_options.fbs',
    command : [
      flatc,
      '--cpp',
      '-o',
      '@OUTPUT@',
      '@INPUT@',
    ],
    install : true,
    install_dir : 'include/api',
  )
)

api_driver_options_helper = declare_dependency(
  sources : [
    'driver_options_helper.cc',
    'driver_options_helper.h',
  ],
  dependencies : [
    api_driver,
    api_driver_options_fbs,
    executable_fbs,
  ],
  include_directories : include_directories('..'),
)

api_driver_factory = declare_dependency(
  sources : [
    'driver_factory.cc',
    'driver_factory.h',
  ],
  dependencies : [
    api_chip,
    api_driver,
    port,
    port_default_port,
    port_default_port_from_tf,
    api_driver_options_fbs,
    executable_fbs,
  ],
  include_directories : include_directories('..'),
)

api_runtime_version = declare_dependency(
  sources : 'runtime_version.h',
)

api_watchdog = declare_dependency(
  sources : [
    'watchdog.cc',
    'watchdog.h',
  ],
  dependencies : [
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_timer,
    port_default_port,
    port_default_port_from_tf,
    threads,
  ],
  include_directories : include_directories('..'),
)
