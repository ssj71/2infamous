#spencer

path = join_paths(get_option('libdir'), 'lv2', 'apc.lv2')
lv2dep = dependency('lv2', required: true)
mdep = meson.get_compiler('c').find_library('m', required: true)

shared_library(
    'ataripunkconsole',
    'apc.c',
    dependencies: [lv2dep, mdep],
    name_prefix: '',
    install: true,
    install_dir: path,
    c_args: '-ffast-math',
    )

install_data(
    'manifest.ttl',
    'apc.ttl',
    install_dir: path,
    )


