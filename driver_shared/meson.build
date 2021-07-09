subdir('time_stamper')

install_headers('registers.h', subdir : 'driver_shared')

driver_shared_registers = declare_dependency(
  sources : 'registers.h',
  dependencies : [
    port,
  ]
)
