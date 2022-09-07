#spencer

path = join_paths(get_option('libdir'), 'lv2', 'ok_saturation.lv2')


#TODO: move this stuff to a shared space
mdep = meson.get_compiler('c').find_library('m', required: true)
rms_calc = static_library('rms_calc','rms_calc.c', dependencies: mdep)

lv2dep = dependency('lv2', required: true)

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


