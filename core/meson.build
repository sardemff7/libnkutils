# Core modules

libexecdir = join_paths(get_option('libexecdir'), major_name)

if not meson.is_subproject()
    install_headers(files(
            'include/nkutils-enum.h',
            'include/nkutils-format-string.h',
            'include/nkutils-colour.h',
            'include/nkutils-xdg-de.h',
            'include/nkutils-xdg-theme.h',
        ),
        subdir: major_name,
    )
endif

nk_inc = include_directories('include')
nk_lib = static_library('nkutils-@0@'.format(major_version), files(
        'include/nkutils-gtk-settings.h',
        'src/enum.c',
        'src/format-string.c',
        'src/colour.c',
        'src/xdg-de.c',
        'src/gtk-settings.c',
        'src/xdg-theme.c',
    ),
    c_args: nk_args,
    include_directories: nk_inc,
    dependencies: nk_deps,
    install: not meson.is_subproject(),
)
nk_src = []

if nk_git_work_tree != ''
    nk_git = find_program('git', required: false)
    if not nk_git.found()
        nk_git = ''
    endif

    nk_git_version = executable('nk-git-version-native', files(
            'src/enum.c',
            'src/format-string.c',
            'src/git-version.c'
        ),
        c_args: nk_args,
        dependencies: dependency('glib-2.0', version: '>= @0@'.format(glib_min_version), native: true),
        include_directories: nk_inc,
        install: false,
        native: true
    )
    nk_src += custom_target('nkutils-git-version.h',
        output: 'nkutils-git-version.h',
        command: [ nk_git_version, 'header', '@OUTPUT@', nk_git_work_tree, nk_git ],
        build_always_stale: true,
        build_by_default: not meson.is_subproject(),
    )
    nkutils_mandepends += custom_target('nkutils-git-version.ent',
        output: 'nkutils-git-version.ent',
        command: [ nk_git_version, 'entity', '@OUTPUT@', nk_git_work_tree, nk_git ],
        build_always_stale: true,
        build_by_default: not meson.is_subproject(),
    )

endif

libnkutils = declare_dependency(link_with: nk_lib, include_directories: nk_inc, dependencies: nk_deps, sources: nk_src)
libnkutils_gtk_settings = nk_lib.extract_objects('src/gtk-settings.c')

executable('nk-format-string-replace', files(
        'src/format-string-example.c'
    ),
    dependencies: libnkutils,
    install: not meson.is_subproject(),
    install_dir: libexecdir
)

executable('nk-xdg-theme-lookup', files(
        'src/xdg-theme-example.c'
    ),
    dependencies: libnkutils,
    install: not meson.is_subproject(),
    install_dir: libexecdir
)

test('libnkutils enum module tests',
    executable('nk-enum.test', files('tests/enum.c'),
        dependencies: libnkutils
    ),
    suite: [ 'enum' ],
    args: [ '--tap' ],
    protocol: 'tap',
)
test('libnkutils format-string module tests',
    executable('nk-format-string.test', files('tests/format-string.c'),
        dependencies: libnkutils
    ),
    suite: [ 'format-string' ],
    args: [ '--tap' ],
    protocol: 'tap',
)
test('libnkutils colour module tests',
    executable('nk-colour.test', files('tests/colour.c'),
        dependencies: libnkutils
    ),
    suite: [ 'colour' ],
    args: [ '--tap' ],
    protocol: 'tap',
)
test('libnkutils xdg-de module tests',
    executable('nk-xdg-de.test', files('tests/xdg-de.c'),
        dependencies: libnkutils
    ),
    suite: [ 'xdg-de' ],
    args: [ '--tap' ],
    protocol: 'tap',
)
test('libnkutils gtk-settings module tests',
    executable('nk-gtk-settings.test', files('tests/gtk-settings.c'),
        objects: libnkutils_gtk_settings,
        c_args: [ '-DSRCDIR="@0@"'.format(meson.current_source_dir()) ],
        dependencies: libnkutils
    ),
    suite: [ 'gtk-settings' ],
    args: [ '--tap' ],
    protocol: 'tap',
)
test('libnkutils xdg-theme module tests',
    executable('nk-xdg-theme.test', files('tests/xdg-theme.c'),
        c_args: [ '-DSRCDIR="@0@"'.format(meson.current_source_dir()) ],
        dependencies: libnkutils
    ),
    suite: [ 'xdg-theme' ],
    args: [ '--tap' ],
    protocol: 'tap',
)
