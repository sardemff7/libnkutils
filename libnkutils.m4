m4_define(_NK_MODULES, [enum token colour])
m4_define(_NK_FEATURES, [token/enum colour/alpha colour/double colour/string])

# auto-enable
m4_define(_nk_dependent_enum, [token/enum])

AC_DEFUN([_NK_MODULE_INIT], [
    _nk_module_[$1]_enable=no
])

AC_DEFUN([_NK_MODULE_CHECK_DEPENDENT], [m4_ifdef([$1], m4_map_args_w($1, [[ -o "x$_nk_module_]m4_translit(], [, [/-], [__])[_enable]" = xyes]))])

AC_DEFUN([_NK_MODULE_CHECK], [
    AM_CONDITIONAL([NK_ENABLE_]m4_toupper([$1]), [test "x$_nk_module_[$1]_enable" = xyes]_NK_MODULE_CHECK_DEPENDENT([_nk_dependent_][$1]))
])

AC_DEFUN([NK_INIT], [
    AC_REQUIRE([PKG_PROG_PKG_CONFIG])

    m4_define([_nk_dir], m4_default([$1], [libnkutils]))
    m4_define([_nk_dir_canon], m4_translit(_nk_dir, [/+-], [___]))

    m4_define([_nk_library_suffix], [])
    m4_define([_NK_LIBRARY_VARIABLE], [])
    m4_ifdef([LT_INIT], [
        m4_define([_nk_library_suffix], [l])
        m4_define([_NK_LIBRARY_VARIABLE], [LT])
    ])

    m4_syscmd([sed ]_nk_dir[/libnkutils.mk.in -e 's:@nk_dir@:]_nk_dir[:g' -e 's:@nk_dir_canon@:]_nk_dir_canon[:g' -e 's:@LIBRARY_VARIABLE@:]_NK_LIBRARY_VARIABLE[:g' -e 's:@library_suffix@:]_nk_library_suffix[:g' -e 's:@config_h@:]m4_default(AH_HEADER, [$(null)])[:g' -e 's:@GLIB_PREFIX@:]m4_default([$2], [GLIB])[:g' > ]_nk_dir[/libnkutils.mk])

    m4_map_args_w(_NK_MODULES, [_NK_MODULE_INIT(], [)])
    AC_CONFIG_COMMANDS_PRE([
        m4_map_args_w(_NK_MODULES, [_NK_MODULE_CHECK(], [)])
    ])

    m4_ifnblank([$3], [NK_ENABLE_MODULES([$3])])
    m4_define([_NK_DOCBOOK_CONDITIONS_VAR], [$4])

    m4_define([GW_INIT])
])

AC_DEFUN([_NK_ENABLE_MODULE], [
    m4_define([_nk_module], [$1])
    m4_define([_nk_feature], [])
    m4_if(m4_index([$1], [/]), [-1], [], [
        m4_define([_nk_module], m4_substr([$1], 0, m4_index([$1], [/])))
        m4_define([_nk_feature], m4_substr([$1], m4_incr(m4_index([$1], [/]))))
    ])
    m4_if(m4_index(_NK_MODULES, _nk_module), [-1], [AC_MSG_ERROR([libnkutils: No ]_nk_module[ module])])
    m4_ifnblank(_nk_feature, m4_if(m4_index(_NK_FEATURES, _nk_feature), [-1], [AC_MSG_ERROR([libnkutils: No ]_nk_feature[ in module ]_nk_module)]))
    [_nk_module_]_nk_module[_enable=yes]
    m4_ifnblank(_nk_feature, [
        [_nk_module_]_nk_module[_]m4_translit(_nk_feature, [-], [_])[_enable=yes]
        m4_ifnblank(_NK_DOCBOOK_CONDITIONS_VAR, _NK_DOCBOOK_CONDITIONS_VAR[="${]_NK_DOCBOOK_CONDITIONS_VAR[};nk_enable_]_nk_module[_]_nk_feature["])
        AC_DEFINE([NK_ENABLE_]m4_toupper(_nk_module)[_]m4_translit(m4_toupper(_nk_feature), [-], [_]), [1], [libnkutils ]_nk_module[ module feature ]_nk_feature)
    ])
])

AC_DEFUN([NK_ENABLE_MODULES], [
    m4_map_args_w([$1], [_NK_ENABLE_MODULE(], [)])
])
