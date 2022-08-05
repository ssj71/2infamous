#spencer

path = join_paths(get_option('libdir'), 'lv2', 'ok_saturation.lv2')
lv2dep = dependency('lv2', required: true)
shared_library('ok_saturation', 'ok_sat.c', dependencies: lv2dep, name_prefix: '', install: true, install_dir: path)
install_data('ok_saturation.ttl','manifest.ttl', install_dir: path)


