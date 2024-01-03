proj_inc = include_directories(
  'include/swrast',
)

proj_src = files(
  'main.cpp',
  'camera.cpp'
)

subdir('swrast')

executable(proj_name, proj_src,
  dependencies : proj_deps,
  include_directories : proj_inc,
)
