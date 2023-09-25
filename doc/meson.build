config_ent = configure_file(input: files('config.ent.in'), output: 'config.ent', configuration: other_conf)

git = find_program('git', required: false)
git_version_ent = custom_target('nkutils-git-version.ent',
    output: 'nkutils-git-version.ent',
    command: [ nk_git_version, 'entity', '@OUTPUT@', git_work_tree, git.found() ? git : '' ],
    build_always_stale: true,
)
xsltproc = [
    find_program('xsltproc'),
    '-o', '@OUTDIR@/',
    '--nonet', '--xinclude',
    '--stringparam', 'man.output.quietly', '1',
    '--stringparam', 'funcsynopsis.style', 'ansi',
    '--path', meson.current_source_dir(),
    '--path', meson.current_build_dir(),
    'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl',
    '@INPUT@'
]
custom_target('nkutils-@0@.5'.format(major_version),
    input: files('manpage.xml'),
    output: 'nkutils-@0@.5'.format(major_version),
    command: xsltproc,
    depend_files: [ config_ent ] + files(
        'nkutils-format-string-format.xml',
        'nkutils-colour-format.xml',
        'nkutils-bindings-format.xml',
    ),
    depends: [ git_version_ent ],
    build_by_default: true,
    install: not meson.is_subproject(),
    install_dir: join_paths(get_option('mandir'), 'man5')
)

gnome = import('gnome')

gtk_doc = dependency('gtk-doc', required: get_option('gtk-doc'))
if gtk_doc.found()
    gnome.gtkdoc('libnkutils',
        dependencies: [ libnkutils, libnkutils_uuid, libnkutils_bindings ],
        namespace: 'nk',
        main_xml: 'libnkutils-docs.xml',
        content_files: [
            'nkutils-format-string-format.xml',
            'nkutils-colour-format.xml',
            'nkutils-bindings-format.xml',
        ],
        install: not meson.is_subproject(),
        install_dir: major_name,
        src_dir: [
            'core',
            'uuid',
            'bindings',
        ],
        ignore_headers: [
            'uuid-internal.h',
            'nkutils-gtk-settings.h',
        ],
    )
endif
