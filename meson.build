#
# libnkutils - Miscellaneous utilities
#
# Copyright © 2011-2016 Quentin "Sardem FF7" Glidic
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

nk_glib_min_version = '2.40'

nk_sources = []
nk_args = [ '-DDATADIR="@0@"'.format(join_paths(get_option('prefix'), get_option('datadir'))) ]
nk_deps = [ dependency('glib-2.0', version: '>= @0@'.format(nk_glib_min_version)) ]
nk_test_deps = [ dependency('gobject-2.0') ]

foreach f : nk_modules
    if f.contains('/')
        nk_modules += f.split('/')[0]
    endif
endforeach

if nk_modules.contains('uuid')
    nk_sources += [
        'src/uuid-internal.h',
        'src/uuid.c',
        'src/nkutils-uuid.h',
    ]
    nk_libuuid = dependency('uuid', required: false)
    nk_apr_util_1 = dependency('apr-util-1', required: false)
    nk_apr_util = dependency('apr-util', required: false)
    if nk_libuuid.found()
        nk_sources += 'src/uuid-libuuid.c'
        nk_deps += nk_libuuid
    elif nk_apr_util_1.found()
        nk_sources += 'src/uuid-apr-util.c'
        nk_deps += nk_apr_util_1
    elif nk_apr_util.found()
        nk_sources += 'src/uuid-apr-util.c'
        nk_deps += nk_apr_util
    else
        error('libnkutils: A UUID library is required')
    endif
endif

if nk_modules.contains('token')
    if nk_modules.contains('token/enum')
        nk_modules += 'enum'
        nk_args += '-DNK_ENABLE_TOKEN_ENUM'
    endif
    nk_sources += [
        'src/token.c',
        'src/nkutils-token.h',
    ]
endif

if nk_modules.contains('xdg-theme')
    nk_modules += 'enum'
    nk_sources += [
        'src/xdg-theme.c',
        'src/nkutils-xdg-theme.h',
    ]
endif

if nk_modules.contains('enum')
    nk_sources += [
        'src/enum.c',
        'src/nkutils-enum.h',
    ]
endif

if nk_modules.contains('colour')
    if nk_modules.contains('colour/alpha')
        nk_args += '-DNK_ENABLE_COLOUR_ALPHA'
    endif
    if nk_modules.contains('colour/double')
        nk_args += '-DNK_ENABLE_COLOUR_DOUBLE'
    endif
    if nk_modules.contains('colour/string')
        nk_args += '-DNK_ENABLE_COLOUR_STRING'
    endif
    nk_sources += [
        'src/colour.c',
        'src/nkutils-colour.h',
    ]
endif

nk_lib = static_library('nkutils', nk_sources, c_args: nk_args, dependencies: nk_deps)
nk_inc = include_directories('src')

libnkutils = declare_dependency(link_with: nk_lib, include_directories: nk_inc, dependencies: nk_deps)


if is_variable('docbook_conditions')
    nkutils_xsltpath = join_paths(meson.current_source_dir(), 'man')
    nkutils_manfiles = files('man/libnkutils-man.xml')

    foreach f : nk_modules
        if f.contains('/')
            docbook_conditions += 'nk_enable_@0@'.format(f.underscorify().to_lower())
        endif
    endforeach
endif

foreach m : [ 'uuid', 'token', 'xdg-theme', 'enum', 'colour' ]
    if nk_modules.contains(m)
        nk_test = executable('nk-@0@.test'.format(m), 'tests/@0@.c'.format(m), dependencies: libnkutils)
        test('libnkutils @0@ module tests'.format(m), nk_test)
    endif
endforeach
