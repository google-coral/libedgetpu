project(
  'libedgetpu',
  'cpp',
  version : '1.0.0',
  default_options : [
    'cpp_std=c++11',
  ]
)

build_type = get_option('buildtype')

project_arguments = [
  '-Wno-sign-compare',
  '-Wno-return-type',
]

if build_type == 'debug'
  project_arguments += '-ggdb'
endif

add_project_arguments(project_arguments, language : 'cpp')

threads = dependency('threads')

absl_strings = dependency('absl_strings')
absl_synchronization = dependency('absl_synchronization')
absl_time = dependency('absl_time')
absl_flags = dependency('absl_flags')
absl_flags_parse = dependency('absl_flags_parse')
absl_flat_hash_map = dependency('absl_flat_hash_map')
absl_optional = dependency('absl_optional')
libusb = dependency('libusb-1.0')
flatbuffers = dependency('flatbuffers')
tensorflow_lite = dependency('tensorflow-lite')

bash = find_program('bash')
flatc = find_program('flatc')

pkgconfig = import('pkgconfig')

is_windows = build_machine.system() == 'windows'
is_linux = build_machine.system() == 'linux'
is_darwin = build_machine.system() == 'darwin'

pci_enabled = get_option('pci').enabled()
usb_enabled = get_option('usb').enabled()

subdir('port')
subdir('executable')
subdir('api')
subdir('driver_shared')
subdir('driver')
subdir('tflite')
