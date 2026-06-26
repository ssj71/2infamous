#spencer

path = join_paths(get_option('libdir'), 'lv2', 'noscillator.lv2')

shared_library(
    'noscillator',
    'nosc.c',
    dependencies: [lv2dep, mdep],
    name_prefix: '',
    install: true,
    install_dir: path,
    c_args: '-ffast-math',
    )

install_data(
    'manifest.ttl',
    'noscillator.ttl',
    install_dir: path,
    )


