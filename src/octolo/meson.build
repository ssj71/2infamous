#spencer

path = join_paths(get_option('libdir'), 'lv2', 'octolo.lv2')

shared_library(
    'octolo',
    'octolo.c',
    dependencies: [lv2dep, mdep],
    name_prefix: '',
    install: true,
    install_dir: path,
    c_args: '-ffast-math',
    )

install_data(
    'manifest.ttl',
    'octolo.ttl',
    'octolo_presets.ttl',
    install_dir: path,
    )


