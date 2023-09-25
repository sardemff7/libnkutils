#
# libnkutils - Miscellaneous utilities
#
# Copyright © 2011-2021 Quentin "Sardem FF7" Glidic
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
    version: '1.0.0',
    meson_version: '>=0.47.0',
    license: 'MIT',
    default_options: [
        'c_std=gnu11',
    ],
)

if meson.is_subproject()
    nk_git_work_tree = get_option('git-work-tree')
else
    nk_git_work_tree = meson.source_root()
endif
nkutils_xsltpaths = [ join_paths(meson.current_source_dir(), 'doc'), join_paths(meson.current_build_dir(), 'core') ]
nkutils_manfiles = files('doc/libnkutils-man.xml')
nkutils_mandepends = []


is_unix = host_machine.system() != 'windows'

major_version = meson.project_version().split('.')[0]
major_name = '@0@-@1@'.format(meson.project_name(), major_version)

glib_min_major='2'
glib_min_minor='40'
glib_min_version='.'.join([glib_min_major, glib_min_minor])

nk_args = [
    '-DSYSCONFDIR="@0@"'.format(join_paths(get_option('prefix'), get_option('sysconfdir'))),
    '-DGLIB_VERSION_MIN_REQUIRED=(G_ENCODE_VERSION(@0@,@1@))'.format(glib_min_major, glib_min_minor),
    '-DNK_EXPORT=',
]


add_project_arguments(
    '-fvisibility=hidden',
    '-I@0@'.format(meson.current_build_dir()),
    language: 'c'
)
add_project_arguments(
    '-fvisibility=hidden',
    '-I@0@'.format(meson.current_build_dir()),
    language: 'c',
    native: true
)

nk_deps = [
    dependency('glib-2.0', version: '>= @0@'.format(glib_min_version)),
    dependency('gio-2.0'),
    dependency('gobject-2.0'),
]

nk_options = []

# Core modules
subdir('core')

# UUID module
uuid = get_option('uuid')
if uuid
    nk_options += [ 'uuid=true' ]
    subdir('uuid')
    if not libnkutils_uuid.found()
        error('UUID module requested, but none of libuuid or apr-util was found')
    endif
endif

# Bindings module
if get_option('bindings')
    nk_options += [ 'bindings=true' ]
    nk_xkbcommon = dependency('xkbcommon', version: '>= 0.4.1', required: get_option('bindings'))
    subdir('bindings')
endif
