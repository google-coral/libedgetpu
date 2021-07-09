HEADERS = [
  'custom_op.h',
  'custom_op_data.h',
  'custom_op_user_data_direct.h',
  'edgetpu_context_direct.h',
  'edgetpu_context_factory.h',
  'edgetpu_delegate_for_custom_op.h',
  'edgetpu_manager_direct.h',
]

install_headers(HEADERS, subdir : 'tflite')

# public refers to tflife/public, not to any notion of API visibility
TFLITE_PUBLIC_HEADERS = [
  'public/edgetpu.h',
  'public/edgetpu_c.h',
]

install_headers(TFLITE_PUBLIC_HEADERS, subdir : 'tflite/public')

LIBEDGETPU_LINK_ARGS = [
  '-Wl,-soname,libedgetpu.so.1',
  '-Wl,--version-script=../@0@'.format(files('public/libedgetpu.lds')[0]),
]

tflite_custom_op_data = declare_dependency(
  sources : [
    'custom_op_data.cc',
    'custom_op_data.h',
  ],
  dependencies : [
    api_chip,
    port,
    flatbuffers,
  ],
)

tflite_public_edgetpu = declare_dependency(
  sources : 'public/edgetpu.h',
  dependencies : [
    tensorflow_lite,
  ],
)

tflite_public_edgetpu_c = declare_dependency(
  sources : 'public/edgetpu_c.h',
  dependencies : [
    tensorflow_lite,
  ],
)

tflite_custom_op = declare_dependency(
  sources : [
    'custom_op.cc',
    'custom_op.h',
  ],
  dependencies : [
    tflite_custom_op_data,
    absl_strings,
    api_layer_information,
    driver_package_registry,
    port,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

tflite_custom_op_user_data_direct = declare_dependency(
  sources : [
    'custom_op_user_data_direct.cc',
    'custom_op_user_data_direct.h',
  ],
  dependencies : [
    tflite_custom_op,
    tflite_custom_op_data,
    absl_flat_hash_map,
    api_driver,
    driver_package_registry,
    port,
  ],
)

tflite_edgetpu_delegate_for_custom_op = declare_dependency(
  sources : [
    'edgetpu_delegate_for_custom_op.cc',
    'edgetpu_delegate_for_custom_op.h',
  ],
  dependencies : [
    port,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

tflite_edgetpu_c = declare_dependency(
  sources : 'edgetpu_c.cc',
  dependencies : [
    tflite_edgetpu_delegate_for_custom_op,
    port,
    tflite_public_edgetpu,
    tflite_public_edgetpu_c,
  ],
)

tflite_edgetpu_context_direct_header = declare_dependency(
  sources : 'edgetpu_context_direct.h',
  dependencies : [
    tflite_custom_op_data,
    tflite_custom_op_user_data_direct,
    api_driver,
    api_driver_factory,
    api_driver_options_fbs,
    api_package_reference,
    driver_driver_factory,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

tflite_edgetpu_manager_direct_header = declare_dependency(
  sources : 'edgetpu_manager_direct.h',
  dependencies : [
    tflite_custom_op_data,
    tflite_custom_op_user_data_direct,
    tflite_edgetpu_context_direct_header,
    api_driver,
    api_driver_factory,
    api_driver_options_fbs,
    api_package_reference,
    driver_driver_factory,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

tflite_edgetpu_context_direct = declare_dependency(
  sources : [
    'edgetpu_context_direct.cc',
    'edgetpu_context_direct.h',
  ],
  dependencies : [
    tflite_custom_op_user_data_direct,
    tflite_edgetpu_manager_direct_header,
    absl_strings,
    api_driver,
    api_driver_factory,
    api_driver_options_fbs,
    api_package_reference,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    tflite_public_edgetpu,
  ],
)

tflite_edgetpu_delegate_for_custom_op_tflite_plugin = declare_dependency(
  sources : 'edgetpu_delegate_for_custom_op_tflite_plugin.cc',
  dependencies : [
    tflite_edgetpu_delegate_for_custom_op,
    absl_strings,
    absl_optional,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

tflite_edgetpu_manager_direct = declare_dependency(
  sources : [
    'edgetpu_manager_direct.cc',
    'edgetpu_manager_direct.h',
  ],
  dependencies : [
    tflite_custom_op_data,
    tflite_custom_op_user_data_direct,
    tflite_edgetpu_context_direct,
    absl_strings,
    api_driver,
    api_driver_factory,
    api_driver_options_fbs,
    api_package_reference,
    api_runtime_version,
    driver_driver_factory,
    port,
    port_std_mutex_lock,
    port_thread_annotations,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

tflite_custom_op_direct = declare_dependency(
  sources : 'custom_op_direct.cc',
  dependencies : [
    tflite_custom_op,
    tflite_custom_op_data,
    tflite_custom_op_user_data_direct,
    tflite_edgetpu_c,
    tflite_edgetpu_context_direct,
    tflite_edgetpu_delegate_for_custom_op_tflite_plugin,
    tflite_edgetpu_manager_direct,
    api_driver,
    port,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)

if usb_enabled and pci_enabled
  deps = [
    tflite_custom_op_direct,
  ]

  if is_windows
    deps += driver_beagle_beagle_all_driver_provider_windows
  else
    deps += driver_beagle_beagle_all_driver_provider_linux
  endif

  tflite_public_edgetpu_internal_direct_all = declare_dependency(
    dependencies : deps,
  )

  tflite_public_libedgetpu_direct_all = library(
    'edgetpu',
    dependencies : [
      tflite_public_edgetpu_internal_direct_all,
    ],
    link_args : LIBEDGETPU_LINK_ARGS,
    install : true,
  )

  pkgconfig.generate(tflite_public_libedgetpu_direct_all)
endif

if pci_enabled and not usb_enabled
  deps = [
    tflite_custom_op_direct,
  ]

  if is_windows
    deps += driver_beagle_beagle_pci_driver_provider_windows
  else
    deps += driver_beagle_beagle_pci_driver_provider_linux
  endif

  tflite_public_edgetpu_internal_direct_pci = declare_dependency(
    dependencies : deps,
  )

  tflite_public_libedgetpu_direct_pci = library(
    'edgetpu',
    dependencies : [
      tflite_public_edgetpu_internal_direct_pci,
    ],
    link_args : LIBEDGETPU_LINK_ARGS,
    install : true,
  )

  pkgconfig.generate(tflite_public_libedgetpu_direct_pci)
endif

if not pci_enabled and not usb_enabled
  tflite_public_libedgetpu_bare = library(
    'edgetpu',
    dependencies : [
      tflite_custom_op_direct,
    ],
    link_args : LIBEDGETPU_LINK_ARGS,
    install : true,
  )

  pkgconfig.generate(tflite_public_libedgetpu_bare)
endif

if usb_enabled and not pci_enabled
  deps = [
      tflite_custom_op_direct,
  ]

  if is_windows
    deps += driver_beagle_beagle_usb_driver_provider_windows
  else
    deps += driver_beagle_beagle_usb_driver_provider
  endif

  tflite_public_edgetpu_internal_direct_usb = declare_dependency(
    dependencies : deps,
  )

  tflite_public_libedgetpu_direct_usb = library(
    'edgetpu',
    dependencies : [
      tflite_public_edgetpu_internal_direct_usb,
    ],
    link_args : LIBEDGETPU_LINK_ARGS,
    install : true,
  )

  pkgconfig.generate(tflite_public_libedgetpu_direct_usb)
endif

if usb_enabled and not pci_enabled
  tflite_public_oss_edgetpu_direct_usb = declare_dependency(
    dependencies : [
      tflite_public_edgetpu_internal_direct_usb,
    ]
  )
endif

if pci_enabled and not usb_enabled
  if is_windows
    tflite_public_oss_edgetpu_direct_pci = declare_dependency(
      dependencies : [
        tflite_public_edgetpu_internal_direct_pci_windows,
      ],
    )
  elif is_linux
    tflite_public_oss_edgetpu_direct_pci = declare_dependency(
      dependencies : [
        tflite_public_edgetpu_internal_direct_pci,
      ],
    )
  endif
endif

if usb_enabled and pci_enabled
  if is_windows
    tflite_public_oss_edgetpu_direct_all = declare_dependency(
      dependencies : [
        tflite_public_edgetpu_internal_direct_all_windows
      ],
    )
  elif is_darwin
    tflite_public_oss_edgetpu_direct_all = declare_dependency(
      dependencies : [
        tflite_public_edgetpu_internal_direct_usb
      ],
    )
  else
    tflite_public_oss_edgetpu_direct_all = declare_dependency(
      dependencies : [
        tflite_public_edgetpu_internal_direct_all
      ],
    )
  endif
endif

tflite_edgetpu_context_factory = declare_dependency(
  sources : [
    'edgetpu_context_factory.cc',
    'edgetpu_context_factory.h',
  ],
  dependencies : [
    tflite_edgetpu_context_direct,
    port,
    tflite_public_edgetpu,
    tensorflow_lite,
  ],
)
