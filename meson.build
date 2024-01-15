project('PGR project - SW rasterizer', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

add_global_arguments('-DGLM_FORCE_SSE2', language : 'cpp')
add_global_arguments('-DGLM_FORCE_ALIGNED', language : 'cpp')

proj_deps = [
  dependency('imguiwrapper', default_options : [
    'glm=enabled',
  ]),
  dependency('ren_utils'),
  dependency('imwidgets'),
]

proj_name = 'main'
subdir('src')
