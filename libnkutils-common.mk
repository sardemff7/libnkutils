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

check_PROGRAMS += \
	$(_libnkutils_tests)

noinst_PROGRAMS += \
	$(_libnkutils_examples)

TESTS += \
	$(_libnkutils_tests)

EXTRA_DIST += \
	%D%/core/src/git-version.c \
	%D%/doc/libnkutils-man.xml \
	%D%/core/tests/gtk-3.0/settings.ini \
	%D%/core/tests/gtk-4.0/settings.ini \
	%D%/core/tests/icons/recursive-theme-test/index.theme \
	%D%/core/tests/icons/recursive-theme-test/test-dir/test-icon.svg \
	$(null)


NKUTILS_CFLAGS = \
	-I$(srcdir)/%D%/bindings/include \
	-I$(srcdir)/%D%/core/include \
	-I$(srcdir)/%D%/uuid/include \
	$(_NKUTILS_INTERNAL_UUID_CFLAGS) \
	$(_NKUTILS_INTERNAL_XKBCOMMON_CFLAGS) \
	$(_NKUTILS_INTERNAL_GIO_CFLAGS) \
	$(_NKUTILS_INTERNAL_GOBJECT_CFLAGS) \
	$(_NKUTILS_INTERNAL_GLIB_CFLAGS)

_NKUTILS_INTERNAL_CFLAGS = \
	-DSRCDIR=\"$(srcdir)/%D%/core\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DNK_EXPORT= \
	$(null)

NKUTILS_LIBS = \
	$(_libnkutils_library) \
	$(_NKUTILS_INTERNAL_UUID_LIBS) \
	$(_NKUTILS_INTERNAL_XKBCOMMON_LIBS) \
	$(_NKUTILS_INTERNAL_GIO_LIBS) \
	$(_NKUTILS_INTERNAL_GOBJECT_LIBS) \
	$(_NKUTILS_INTERNAL_GLIB_LIBS)

NKUTILS_XSLTPROCFLAGS = \
	--path "$(srcdir)/%D%/man/"

NKUTILS_MANFILES = \
	%D%/man/libnkutils-man.xml


_libnkutils_sources =
_libnkutils_examples =
_libnkutils_tests =

if NK_ENABLE_UUID_LIBUUID
_libnkutils_sources += \
	%D%/uuid/src/uuid-libuuid.c \
	%D%/uuid/src/uuid-internal.h \
	%D%/uuid/src/uuid-nosystemd.c \
	%D%/uuid/src/uuid.c \
	%D%/uuid/include/nkutils-uuid.h

_NKUTILS_INTERNAL_UUID_CFLAGS = \
	$(_NKUTILS_INTERNAL_UUID_LIBUUID_CFLAGS)

_NKUTILS_INTERNAL_UUID_LIBS = \
	$(_NKUTILS_INTERNAL_UUID_LIBUUID_LIBS)

_libnkutils_tests += \
	%D%/uuid/tests/uuid.test
else
if NK_ENABLE_UUID_APR_UTIL
_libnkutils_sources += \
	%D%/uuid/src/uuid-apr-util.c \
	%D%/uuid/src/uuid-internal.h \
	%D%/uuid/src/uuid-nosystemd.c \
	%D%/uuid/src/uuid.c \
	%D%/uuid/include/nkutils-uuid.h

_NKUTILS_INTERNAL_UUID_CFLAGS = \
	$(_NKUTILS_INTERNAL_UUID_APR_UTIL_CFLAGS)

_NKUTILS_INTERNAL_UUID_LIBS = \
	$(_NKUTILS_INTERNAL_UUID_APR_UTIL_LIBS)

_libnkutils_tests += \
	%D%/uuid/tests/uuid.test
endif
endif

if NK_ENABLE_ENUM
_libnkutils_sources += \
	%D%/core/src/enum.c \
	%D%/core/include/nkutils-enum.h

_libnkutils_tests += \
	%D%/core/tests/enum.test
endif

if NK_ENABLE_FORMAT_STRING
_libnkutils_sources += \
	%D%/core/src/format-string.c \
	%D%/core/include/nkutils-format-string.h

_libnkutils_examples += \
	%D%/nk-format-string-replace

_libnkutils_tests += \
	%D%/core/tests/format-string.test
endif

if NK_ENABLE_COLOUR
_libnkutils_sources += \
	%D%/core/src/colour.c \
	%D%/core/include/nkutils-colour.h

_libnkutils_tests += \
	%D%/core/tests/colour.test
endif

if NK_ENABLE_GTK_SETTINGS
_libnkutils_sources += \
	%D%/core/src/gtk-settings.c \
	%D%/core/include/nkutils-gtk-settings.h

_libnkutils_tests += \
	%D%/core/tests/gtk-settings.test
endif

if NK_ENABLE_XDG_DE
_libnkutils_sources += \
	%D%/core/src/xdg-de.c \
	%D%/core/include/nkutils-xdg-de.h

_libnkutils_tests += \
	%D%/core/tests/xdg-de.test
endif

if NK_ENABLE_XDG_THEME
_libnkutils_sources += \
	%D%/core/src/xdg-theme.c \
	%D%/core/include/nkutils-xdg-theme.h

_libnkutils_examples += \
	%D%/nk-xdg-theme-lookup

_libnkutils_tests += \
	%D%/core/tests/xdg-theme.test
endif

if NK_ENABLE_BINDINGS
_libnkutils_sources += \
	%D%/bindings/src/bindings.c \
	%D%/bindings/include/nkutils-bindings.h

_libnkutils_tests += \
	%D%/bindings/tests/bindings.test
endif


#
# Examples
#

# format-string
%C%_nk_format_string_replace_SOURCES = \
	%D%/core/src/format-string-example.c

%C%_nk_format_string_replace_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_nk_format_string_replace_LDADD = \
	$(NKUTILS_LIBS)

# xdg-theme
%C%_nk_xdg_theme_lookup_SOURCES = \
	%D%/core/src/xdg-theme-example.c

%C%_nk_xdg_theme_lookup_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_nk_xdg_theme_lookup_LDADD = \
	$(NKUTILS_LIBS)


#
# Tests
#

# enum
%C%_core_tests_enum_test_SOURCES = \
	%D%/core/tests/enum.c

%C%_core_tests_enum_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_core_tests_enum_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# format-string
%C%_core_tests_format_string_test_SOURCES = \
	%D%/core/tests/format-string.c

%C%_core_tests_format_string_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_core_tests_format_string_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# colour
%C%_core_tests_colour_test_SOURCES = \
	%D%/core/tests/colour.c

%C%_core_tests_colour_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_core_tests_colour_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# uuid
%C%_uuid_tests_uuid_test_SOURCES = \
	%D%/uuid/tests/uuid.c

%C%_uuid_tests_uuid_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_uuid_tests_uuid_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# gtk-settings
%C%_core_tests_gtk_settings_test_SOURCES = \
	%D%/core/tests/gtk-settings.c

%C%_core_tests_gtk_settings_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_core_tests_gtk_settings_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# xdg-de
%C%_core_tests_xdg_de_test_SOURCES = \
	%D%/core/tests/xdg-de.c

%C%_core_tests_xdg_de_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_core_tests_xdg_de_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# xdg-theme
%C%_core_tests_xdg_theme_test_SOURCES = \
	%D%/core/tests/xdg-theme.c

%C%_core_tests_xdg_theme_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_core_tests_xdg_theme_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# bindings
%C%_bindings_tests_bindings_test_SOURCES = \
	%D%/bindings/tests/bindings.c

%C%_bindings_tests_bindings_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_CFLAGS)

%C%_bindings_tests_bindings_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)
