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


AC_DEFUN([NK_INIT], [
    AC_REQUIRE([AC_HEADER_STDC])
    AC_REQUIRE([PKG_PROG_PKG_CONFIG])

    nk_glib_min_version="2.40"

    AC_CONFIG_COMMANDS_PRE([
        AS_IF([test x${ac_cv_header_string_h} != xyes], [AC_MSG_ERROR([libnkutils: string.h is required])])
        PKG_CHECK_MODULES([_NKUTILS_INTERNAL_GLIB], [glib-2.0 >= ${nk_glib_min_version}])
        PKG_CHECK_MODULES([_NKUTILS_INTERNAL_TEST], [gobject-2.0])
    ])

    AM_XSLTPROCFLAGS="${AM_XSLTPROCFLAGS} "'${NKUTILS_XSLTPROCFLAGS}'

    m4_define([NK_INIT])
])
