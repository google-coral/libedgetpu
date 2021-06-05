HEADERS = [
  'time_stamper.h',
  'driver_time_stamper.h',
  'time_stamper_factory.h',
  'driver_time_stamper_factory.h'
]

install_headers(HEADERS, subdir : 'driver_shared/time_stamper')

driver_shared_time_stamper = declare_dependency(
  sources : 'time_stamper.h',
  dependencies : [
    port,
  ],
)

driver_shared_time_stamper_driver_time_stamper = declare_dependency(
  sources : [
    'driver_time_stamper.cc',
    'driver_time_stamper.h',
  ],
  dependencies : [
    driver_shared_time_stamper,
    port,
  ],
  include_directories : include_directories('../..'),
)

driver_shared_time_stamper_time_stamper_factory = declare_dependency(
  sources : 'time_stamper_factory.h',
  dependencies : [
    driver_shared_time_stamper,
    port,
  ],
)

driver_shared_time_stamper_driver_time_stamper_factory = declare_dependency(
  sources : 'driver_time_stamper_factory.h',
  dependencies : [
    driver_shared_time_stamper_driver_time_stamper,
    driver_shared_time_stamper,
    driver_shared_time_stamper_time_stamper_factory,
    port,
  ],
)
