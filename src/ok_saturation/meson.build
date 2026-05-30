#spencer

path = join_paths(get_option('libdir'), 'lv2', 'ok_saturation.lv2')


#TODO: move this stuff to a shared space
rms_calc = static_library('rms_calc','rms_calc.c', dependencies: mdep)

shared_library(
    'ok_saturation',
    'ok_sat.c',
    dependencies: lv2dep,
    link_with: rms_calc,
    c_args: '-ffast-math',
    name_prefix: '',
    install: true,
    install_dir: path,
    )

install_data(
    'ok_saturation.ttl',
    'manifest.ttl',
    install_dir: path,
    )


