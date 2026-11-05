project('libchstr', 'c',
  version: '0.2.0',
  default_options: ['c_std=gnu17', 'warning_level=3'],
)

cc = meson.get_compiler('c')

dep_unistring = cc.find_library('unistring')

chstringlib = library(
  'chstr',
  'src/chstr.c',
  dependencies: dep_unistring,
  install: false,
)

dep_chstring = declare_dependency(
  link_with: chstringlib,
  dependencies: dep_unistring,
  include_directories: include_directories('.'),
)

tests = executable(
  'tests',
  'tests.c',
  dependencies: [dependency('check'), dep_chstring],
)

test('tests', tests)
