port_from_tf_srcs = [
  'logging.cc',
  'status.cc',
  'statusor.cc',
]

port_from_tf_hdrs = [
  'array_slice.h',
  'array_slice_internal.h',
  'casts.h',
  'errors.h',
  'logging.h',
  'macros.h',
  'math_util.h',
  'ptr_util.h',
  'status.h',
  'statusor.h',
  'statusor_internals.h',
]

install_headers(port_from_tf_hdrs, subdir : 'port/default/port_from_tf')

port_default_port_from_tf = declare_dependency(
  sources : port_from_tf_srcs + port_from_tf_hdrs,
  dependencies : [
    port_default_port_base,
  ],
  include_directories : include_directories('../../..'),
)

# XXX: this doesn't exist, not sure how bazel is building it
# library('status_test_util', 'status_test_util.h', dependencies : [port_from_tf])

install_headers('thread_annotations.h', subdir : 'port/default/port_from_tf')

port_default_port_from_tf_thread_annotations = declare_dependency(
  sources : 'thread_annotations.h',
)
