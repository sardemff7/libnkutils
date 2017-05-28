AC_PREREQ([2.65])
AC_INIT([nkutils],[0])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_SRCDIR([src/token.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.aux])

AM_INIT_AUTOMAKE([1.14 foreign subdir-objects parallel-tests dist-xz no-dist-gzip tar-ustar])
AM_SILENT_RULES([yes])

AC_ARG_VAR([XSLTPROC], [The xsltproc executable])

AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT([disable-static pic-only])
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([XSLTPROC], [xsltproc])
if test -z "${XSLTPROC}"; then
    AC_MSG_ERROR([xsltproc not found])
fi

AM_DOCBOOK_CONDITIONS=""

NK_INIT(_NK_MODULES)

AC_SUBST([AM_DOCBOOK_CONDITIONS])

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
