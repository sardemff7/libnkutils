# Bindings module
if nk_xkbcommon.version().version_compare('>= 0.7.0')
    nk_args += '-DNK_XKBCOMMON_HAS_COMPOSE'
    nk_args += '-DNK_XKBCOMMON_HAS_CONSUMED2'
elif nk_xkbcommon.version().version_compare('>= 0.5.0')
    nk_args += '-DNK_XKBCOMMON_HAS_COMPOSE'
endif

if not meson.is_subproject()
    install_headers(files(
            'include/nkutils-bindings.h',
        ),
        subdir: major_name,
    )
endif

nk_inc_bindings = [ nk_inc, include_directories('include') ]
nk_deps_bindings = [ nk_xkbcommon, libnkutils ]
nk_lib = static_library('nkutils-bindings-@0@'.format(major_version), files('src/bindings.c'),
    c_args: nk_args,
    objects: libnkutils_gtk_settings,
    include_directories: nk_inc_bindings,
    dependencies: nk_deps_bindings,
    install: not meson.is_subproject(),
)
libnkutils_bindings = declare_dependency(link_with: nk_lib, include_directories: nk_inc_bindings, dependencies: nk_deps_bindings)

nk_xkeyboard_config = dependency('xkeyboard-config', required: false)
if nk_xkeyboard_config.found()
    test('libnkutils bindings module tests',
        executable('nk-bindings.test', files('tests/bindings.c'),
            dependencies: [ libnkutils_bindings, libnkutils ]
        ),
        suite: [ 'bindings' ],
        args: [ '--tap' ],
        protocol: 'tap',
    )
endif
