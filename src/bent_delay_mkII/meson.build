#spencer

path = join_paths(get_option('libdir'), 'lv2', 'bent_delay_mkII.lv2')
lv2dep = dependency('lv2', required: true)

shared_library(
    'bent_delay_mkII',
    'randlfo.c',
    'bentdly.c',
    dependencies: lv2dep,
    name_prefix: '',
    install: true,
    install_dir: path,
    c_args: '-ffast-math',
    )

install_data(
    'manifest.ttl',
    'bent_delay_mkII.ttl',
    install_dir: path,
    )


