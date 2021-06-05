executable_fbs = declare_dependency(
  sources : custom_target(
    'executable_fbs',
    output : 'executable',
    input : 'executable.fbs',
    command : [
      flatc,
      '--cpp',
      '--gen-object-api',
      '--force-empty',
      '--gen-mutable',
      '-o',
      '@OUTPUT@',
      '@INPUT@',
    ],
    install : true,
    install_dir : 'include',
  )
)
