project('perlovka-filter-op', 'c', version : '0.1')

gegl = dependency('gegl-0.4', required : true)

op3 = shared_library('perlovka',
                     'src/gegl_plugin.c',
                     dependencies : [gegl],
                     name_prefix : '')
