dnl
dnl libnkutils - Miscellaneous utilities
dnl
dnl Copyright © 2011-2015 Quentin "Sardem FF7" Glidic
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining a copy
dnl of this software and associated documentation files (the "Software"), to deal
dnl in the Software without restriction, including without limitation the rights
dnl to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
dnl copies of the Software, and to permit persons to whom the Software is
dnl furnished to do so, subject to the following conditions:
dnl
dnl The above copyright notice and this permission notice shall be included in
dnl all copies or substantial portions of the Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
dnl IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
dnl FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
dnl AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
dnl LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
dnl OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
dnl THE SOFTWARE.
dnl


# NK_INIT([modules])
#     modules                       A list of modules to enable (shorthand for NK_ENABLE_MODULES)
AC_DEFUN([NK_INIT], [
    AC_REQUIRE([AC_HEADER_STDC])
    AC_REQUIRE([PKG_PROG_PKG_CONFIG])

    nk_glib_min_version="2.40"

    m4_map_args_w(_NK_MODULES, [_NK_MODULE_INIT(], [)])
    AC_CONFIG_COMMANDS_PRE([
        AS_IF([test x${ac_cv_header_string_h} != xyes], [AC_MSG_ERROR([libnkutils: string.h is required])])
        PKG_CHECK_MODULES([_NKUTILS_INTERNAL_GLIB], [glib-2.0 >= ${nk_glib_min_version}])
        PKG_CHECK_MODULES([_NKUTILS_INTERNAL_TEST], [gobject-2.0])
        m4_map_args_w(_NK_MODULES, [_NK_MODULE_CHECK(], [)])
    ])

    m4_ifnblank([$1], [NK_ENABLE_MODULES([$1])])

    m4_define([NK_INIT])
])

# NK_ENABLE_MODULES(modules)
#     modules  A list of modules to enable
AC_DEFUN([NK_ENABLE_MODULES], [
    m4_map_args_w([$1], [_NK_ENABLE_MODULE(], [)])
])

m4_define([_NK_MODULES], [enum token colour])
m4_define([_NK_FEATURES], [token/enum colour/double colour/string])


# auto-enable
m4_define([_nk_dependent_enum], [token/enum])



AC_DEFUN([_NK_MODULE_INIT], [
    nk_module_[$1]_enable=no
])

AC_DEFUN([_NK_MODULE_CHECK], [
    AM_CONDITIONAL([NK_ENABLE_]m4_toupper([$1]), [test x${nk_module_[$1]_enable} = xyes]_NK_MODULE_CHECK_DEPENDENT([_nk_dependent_][$1]))
])

AC_DEFUN([_NK_MODULE_CHECK_DEPENDENT], [m4_ifdef([$1], m4_map_args_w($1, [[ -o x${nk_module_]m4_translit(], [, [/], [_])[_enable} = xyes]]))])


AC_DEFUN([_NK_ENABLE_MODULE], [
    m4_if(m4_index([$1], [/]), [-1], [
        _NK_ENABLE_MODULE_INTERNAL([$1])
    ], [
        _NK_ENABLE_MODULE_INTERNAL(m4_substr([$1], 0, m4_index([$1], [/])), m4_substr([$1], m4_incr(m4_index([$1], [/]))), [$1], m4_translit([$1], [/], [_]))
    ])
])

AC_DEFUN([_NK_ENABLE_MODULE_INTERNAL], [
    m4_if(m4_index(_NK_MODULES, [$1]), [-1], [AC_MSG_ERROR([libnkutils: No ][$1][ module])])
    [nk_module_][$1][_enable=yes]

    m4_ifnblank([$2], [
        m4_if(m4_index(_NK_FEATURES, [$3]), [-1], [AC_MSG_ERROR([libnkutils: No ][$2][ in module ][$1])])
        [_nk_module_][$4][_enable=yes]
        AC_DEFINE([NK_ENABLE_]m4_toupper([$4]), [1], [libnkutils ][$1][ module feature ][$2])
    ])
])
