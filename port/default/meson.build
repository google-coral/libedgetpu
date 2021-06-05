port_base_hdrs = [
  'error_codes.h',
  'strcat.h',
  'unreachable.h',
]

if is_windows
  port_base_hdrs += ['unreachable_windows.h']
else
  port_base_hdrs += ['unreachable_default.h']
endif

install_headers(port_base_hdrs, subdir : 'port/default')

port_default_port_base = declare_dependency(
  sources : port_base_hdrs,
  include_directories : include_directories('../..'),
)

subdir('port_from_tf')

install_headers('thread_annotations.h', subdir : 'port/default')

port_default_thread_annotations = declare_dependency(
  sources : 'thread_annotations.h',
  dependencies : [
    port_default_port_from_tf_thread_annotations,
  ],
  include_directories : include_directories('../..'),
)

port_srcs = [
  'status_macros.cc',
  'stringprintf.cc',
]

port_hdrs = [
  'aligned_malloc.h',
  'array_slice.h',
  'builddata.h',
  'casts.h',
  'error_codes.h',
  'errors.h',
  'integral_types.h',
  'logging.h',
  'macros.h',
  'mutex.h',
  'math_util.h',
  'ptr_util.h',
  'status.h',
  'status_macros.h',
  'statusor.h',
  'strcat.h',
  'stringprintf.h',
]

if is_windows
  port_hdrs += ['aligned_malloc_windows.h']
else
  port_hdrs += ['aligned_malloc_default.h']
endif

install_headers(port_hdrs, subdir : 'port/default')

port_default_port = declare_dependency(
  sources : port_srcs + port_hdrs,
  dependencies : [
    port_default_thread_annotations,
    port_default_port_from_tf,
  ],
  include_directories : include_directories('../..'),
)

install_headers('semaphore.h', subdir : 'port/default')

port_default_semaphore = declare_dependency(
  sources : 'semaphore.h',
  dependencies : [
    absl_synchronization,
    absl_time,
    threads,
  ],
)

install_headers('strcat.h', subdir : 'port/default')

port_default_strcat = declare_dependency(
  sources : 'strcat.h',
)

unreachable_hdrs = [
  'unreachable.h',
]

if is_windows
  port_hdrs += ['unreachable_windows.h']
else
  port_hdrs += ['unreachable_default.h']
endif

install_headers('unreachable.h', subdir : 'port/default')

port_default_unreachable = declare_dependency(
  sources : unreachable_hdrs,
)

install_headers('cleanup.h', subdir : 'port/default')

port_default_cleanup = declare_dependency(
  sources : 'cleanup.h',
  dependencies : [
    port_default_port
  ],
)

install_headers('memory_barriers.h', subdir : 'port/default')

port_default_memory_barriers = declare_dependency(
  sources : 'memory_barriers.h',
)
