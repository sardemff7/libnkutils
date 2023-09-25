# UUID module

nk_libuuid = dependency('uuid', required: false)
nk_libsystemd = dependency('libsystemd', required: false)
nk_sources = files(
    'src/uuid.c',
    'src/uuid-internal.h',
)
if nk_libuuid.found()
    nk_sources += files('src/uuid-libuuid.c')
    nk_deps += nk_libuuid
else
    nk_libuuid = dependency('apr-util-1', required: false)
    if nk_libuuid.found()
        nk_sources += files('src/uuid-apr-util.c')
    else
        nk_libuuid = dependency('apr-util', required: false)
        if nk_libuuid.found()
            nk_sources += files('src/uuid-apr-util.c')
            nk_deps += nk_libuuid
        else
            nk_libuuid = disabler()
        endif
    endif
endif
if nk_libsystemd.found()
    nk_sources += files('src/uuid-libsystemd.c')
    nk_deps += nk_libsystemd
else
    nk_sources += files('src/uuid-nosystemd.c')
endif
if not nk_libuuid.found()
    if uuid.enabled()
        error('UUID module requested, but none of libuuid or apr-util was found')
    else
        libnkutils_uuid = disabler()
    endif
else
    if not meson.is_subproject()
        install_headers(files(
                'include/nkutils-uuid.h',
            ),
            subdir: major_name,
        )
    endif
    nk_inc_uuid = [ nk_inc, include_directories('include') ]
    nk_deps_uuid = [ nk_libuuid, nk_deps ]
    nk_lib = static_library('nkutils-uuid-@0@'.format(major_version),  nk_sources,
        c_args: nk_args,
        include_directories: nk_inc_uuid,
        dependencies: nk_deps_uuid,
        install: not meson.is_subproject(),
    )
    libnkutils_uuid = declare_dependency(link_with: nk_lib, include_directories: nk_inc_uuid, dependencies: nk_deps_uuid)

    test('libnkutils uuid module tests',
        executable('nk-uuid.test', files('tests/uuid.c'),
            dependencies: libnkutils_uuid
        ),
        suite: [ 'uuid' ],
        args: [ '--tap' ],
        protocol: 'tap',
    )
endif
