subdir('default')

port_hdrs = [
  'aligned_malloc.h',
  'array_slice.h',
  'builddata.h',
  'casts.h',
  'cleanup.h',
  'defs.h',
  'errors.h',
  'gflags.h',
  'integral_types.h',
  'logging.h',
  'macros.h',
  'math_util.h',
  'openssl.h',
  'ptr_util.h',
  'status.h',
  'status_macros.h',
  'statusor.h',
  'stringprintf.h',
  'time.h',
  'unreachable.h',
]

HEADERS = port_hdrs + [
  'blocking_counter.h',
  'mutex.h',
  'posix_time.h',
  'semaphore.h',
  'shared_mutex.h',
  'std_mutex_lock.h',
  'string_util.h',
  'thread_annotations.h',
  'tracing.h',
]

defines = []

if get_option('darwinn') == 'portable'
  defines += ['-DDARWINN_PORT_DEFAULT']
elif get_option('darwinn_firmware')
  defines += ['-DDARWINN_PORT_FIRMWARE']
else
  defines += ['-DDARWINN_PORT_GOOGLE3']
endif

port_aligned_malloc_deps = []
port_math_util_deps = []
port_macros_deps = []
port_logging_deps = []
port_integral_types_deps = []

port_deps = [
  absl_flags_parse,
  absl_flags,
  absl_strings,
  port_default_cleanup,
  port_default_unreachable,
  port_default_port_from_tf,
]

if get_option('darwinn') == 'portable'
  port_deps += [port_default_port]
  port_aligned_malloc_deps += [port_default_port]
  port_math_util_deps += [port_default_port]
  port_macros_deps += [port_default_port]
  port_logging_deps += [port_default_port]
  port_integral_types_deps += [port_default_port]
endif

port = declare_dependency(
  sources : port_hdrs,
  dependencies : port_deps,
  compile_args : defines,
)

port_aligned_malloc_hdrs = ['aligned_malloc.h', 'defs.h']

port_aligned_malloc = declare_dependency(
  sources : port_aligned_malloc_hdrs,
  dependencies : port_aligned_malloc_deps,
  compile_args : defines,
)

port_math_util_hdrs = [
  'defs.h',
  'math_util.h',
]

port_math_util = declare_dependency(
  sources : port_math_util_hdrs,
  compile_args : defines,
)


port_unreachable = declare_dependency(
  sources : 'unreachable.h',
  dependencies : [
    port_default_unreachable,
  ],
  compile_args : defines,
)

port_macros = declare_dependency(
  sources : [
    'defs.h',
    'macros.h',
  ],
  dependencies : port_macros_deps,
  compile_args : defines,
)

port_logging = declare_dependency(
  sources : [
    'defs.h',
    'logging.h',
  ],
  dependencies : port_logging_deps,
  compile_args : defines,
)

port_thread_annotations = declare_dependency(
  sources : [
    'defs.h',
    'thread_annotations.h',
  ],
  dependencies : [
    port_default_thread_annotations,
  ],
  compile_args : defines,
)

port_integral_types = declare_dependency(
  sources : [
    'defs.h',
    'integral_types.h',
  ],
  dependencies : port_integral_types_deps,
  compile_args : defines,
)

port_std_mutex_lock = declare_dependency(
  sources : 'std_mutex_lock.h',
  dependencies : [
    port,
    port_thread_annotations,
  ],
  compile_args : defines,
)

port_shared_mutex = declare_dependency(
  sources : [
    'shared_mutex.cc',
    'shared_mutex.h',
  ],
  dependencies : [
    port,
    port_thread_annotations,
  ],
  compile_args : defines,
  include_directories : include_directories('..'),
)

port_mutex_deps = []

if get_option('darwinn') == 'portable'
  port_mutex_deps += [port, port_thread_annotations]
endif

port_mutex = declare_dependency(
  sources : [
    'defs.h',
    'mutex.h',
  ],
  dependencies : port_mutex_deps,
  compile_args : defines,
)

port_semaphore_deps = []

if get_option('darwinn') == 'portable'
  port_semaphore_deps += [port, port_thread_annotations]
endif

port_semaphore = declare_dependency(
  sources : ['defs.h', 'semaphore.h'],
  dependencies : port_semaphore_deps,
  compile_args : defines,
)

port_blocking_counter = declare_dependency(
  sources : [
    'blocking_counter.cc',
    'blocking_counter.h',
  ],
  dependencies : [
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    port_default_port_from_tf,
  ],
  include_directories : include_directories('..'),
)

port_tracing = declare_dependency(
  sources : 'tracing.h',
  compile_args : defines,
)

port_string_util = declare_dependency(
  sources : 'string_util.h',
  compile_args : defines,
  dependencies : [
    port_default_strcat,
  ],
)

timer_srcs = []
timer_hdrs = []

if is_windows
  timer_srcs += ['timer_windows.cc']
  timer_hdrs += ['timer_windows.h']
else
  timer_srcs += ['timer_portable.cc']
  timer_hdrs += ['timer_portable.h']
endif

HEADERS += timer_hdrs

port_timer = declare_dependency(
  sources : timer_srcs + timer_hdrs,
  dependencies : [
    port,
    port_default_port,
    port_default_port_from_tf,
  ],
  include_directories : include_directories('..'),
)

port_posix_time = declare_dependency(
  sources : [
    'posix_time.cc',
    'posix_time.h',
  ],
  dependencies : [
    port_integral_types,
  ],
  include_directories : include_directories('..'),
)

port_fileio_srcs = []
port_fileio_hdrs = ['fileio.h']

if is_windows
  port_fileio_srcs += ['fileio_windows.cc']
  port_fileio_hdrs += ['fileio_windows.h']
else
  port_fileio_hdrs += ['fileio_linux.h']
endif

HEADERS += port_fileio_hdrs

port_fileio = declare_dependency(
  sources : port_fileio_srcs + port_fileio_hdrs,
  dependencies : [
    port,
  ],
  include_directories : include_directories('..'),
)

port_status_macros_deps = []

if get_option('darwinn') == 'portable'
  port_status_macros_deps += [port_default_port]
else
  error('cannot compile port_status_macros when darwinn_portable is false')
endif

port_status_macros = declare_dependency(
  sources : [
    'defs.h',
    'status_macros.h',
  ],
  dependencies : port_status_macros_deps,
  include_directories : include_directories('..'),
  compile_args : defines,
)

install_headers(HEADERS, subdir : 'port')
