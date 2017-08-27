#
# libnkutils - Miscellaneous utilities
#
# Copyright © 2011-2017 Quentin "Sardem FF7" Glidic
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

project('libnkutils', 'c',
    license: 'MIT',
    default_options: [
        'c_std=gnu11',
        'warning_level=3',
    ],
)

nk_glib_min_version = '2.40'

nk_options = []

nk_modules = [
    'uuid',
    'token',
    'xdg-theme',
    'enum',
    'colour',
    'bindings',
]
nk_dependencies = [
    [ 'token', 'enum' ],
    [ 'xdg-theme', 'enum' ],
    [ 'bindings', 'enum' ],
]

nk_sources = []
nk_args = [ '-DDATADIR="@0@"'.format(join_paths(get_option('prefix'), get_option('datadir'))) ]
nk_deps = [ dependency('glib-2.0', version: '>= @0@'.format(nk_glib_min_version)) ]

nkutils_xsltpath = join_paths(meson.current_source_dir(), 'man')
nkutils_manfiles = files('man/libnkutils-man.xml')
nkutils_docbook_conditions = []

foreach m : nk_modules
    if get_option(m)
        nk_options += '@0@=true'.format(m)
    endif
    set_variable(m.underscorify(), get_option(m) or not meson.is_subproject())
endforeach

foreach d : nk_dependencies
    if get_option(d[0])
        set_variable(d[1].underscorify(), true)
    endif
endforeach

foreach m : nk_modules
    if get_variable(m.underscorify())
        nk_sources += [
            'src/@0@.c'.format(m),
            'src/nkutils-@0@.h'.format(m),
        ]
    endif
endforeach

if uuid
    nk_libuuid = dependency('uuid', required: false)
    if nk_libuuid.found()
        nk_sources += 'src/uuid-libuuid.c'
        nk_deps += nk_libuuid
    else
        nk_apr_util_1 = dependency('apr-util-1', required: false)
        if nk_apr_util_1.found()
            nk_sources += 'src/uuid-apr-util.c'
            nk_deps += nk_apr_util_1
        else
            nk_apr_util = dependency('apr-util', required: false)
            if nk_apr_util.found()
                nk_sources += 'src/uuid-apr-util.c'
                nk_deps += nk_apr_util
            else
                error('libnkutils: A UUID library is required')
            endif
        endif
    endif
    nk_sources += 'src/uuid-internal.h'
endif

if xdg_theme
    nk_deps += [ dependency('gio-2.0'), dependency('gobject-2.0') ]
endif

if bindings
    nk_xkbcommon = dependency('xkbcommon', version: '>= 0.4.1')
    nk_deps += nk_xkbcommon
    if nk_xkbcommon.version().version_compare('>= 0.7.0')
        nk_args += '-DNK_XKBCOMMON_HAS_COMPOSE'
        nk_args += '-DNK_XKBCOMMON_HAS_CONSUMED2'
    elif nk_xkbcommon.version().version_compare('>= 0.5.0')
        nk_args += '-DNK_XKBCOMMON_HAS_COMPOSE'
    endif
endif

nk_lib = static_library('nkutils', nk_sources, c_args: nk_args, dependencies: nk_deps)
nk_inc = include_directories('src')

libnkutils = declare_dependency(link_with: nk_lib, include_directories: nk_inc, dependencies: nk_deps)


if token
    executable('nk-token-replace', [ 'src/token-example.c' ], dependencies: libnkutils)
endif

foreach m : nk_modules
    if get_variable(m.underscorify())
        nk_test = executable('nk-@0@.test'.format(m), 'tests/@0@.c'.format(m), c_args: nk_args, dependencies: libnkutils)
        test('libnkutils @0@ module tests'.format(m), nk_test)
    endif
endforeach
