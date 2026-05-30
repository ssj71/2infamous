#spencer

path = join_paths(get_option('libdir'), 'lv2', 'bent_delay_mkII.lv2')

shared_library(
    'bent_delay_mkII',
    'randlfo.c',
    'envfollow.c',
    'bentdly.c',
    dependencies: [lv2dep, mdep],
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


