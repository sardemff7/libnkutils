#
# libnkutils - Miscellaneous utilities
#
# Copyright © 2011-2015 Quentin "Sardem FF7" Glidic
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

TESTS += \
	$(_libnkutils_tests)

EXTRA_DIST += \
	%D%/man/libnkutils-man.xml


NKUTILS_CFLAGS = \
	-I$(srcdir)/%D%/src \
	$(_NKUTILS_INTERNAL_GLIB_CFLAGS)

NKUTILS_LIBS = \
	$(_libnkutils_library)
	$(_NKUTILS_INTERNAL_GLIB_LIBS)

NKUTILS_XSLTPROCFLAGS = \
	--path "$(srcdir)/%D%/man/"

NKUTILS_MANFILES = \
	%D%/man/libnkutils-man.xml


_libnkutils_sources = \
	%D%/src/nkutils-enum.h \
	%D%/src/enum.c \
	%D%/src/nkutils-token.h \
	%D%/src/token.c \
	%D%/src/nkutils-colour.h \
	%D%/src/colour.c \
	$(null)

_libnkutils_tests = \
	%D%/tests/enum.test \
	%D%/tests/token.test \
	%D%/tests/colour.test \
	$(null)


#
# Tests
#

# enum
%C%_tests_enum_test_SOURCES = \
	%D%/tests/enum.c

%C%_tests_enum_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_TEST_CFLAGS)

%C%_tests_enum_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# token
%C%_tests_token_test_SOURCES = \
	%D%/tests/token.c

%C%_tests_token_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_TEST_CFLAGS)

%C%_tests_token_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)

# colour
%C%_tests_colour_test_SOURCES = \
	%D%/tests/colour.c

%C%_tests_colour_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(NKUTILS_CFLAGS) \
	$(_NKUTILS_INTERNAL_TEST_CFLAGS)

%C%_tests_colour_test_LDADD = \
	$(NKUTILS_LIBS) \
	$(_NKUTILS_INTERNAL_TEST_LIBS)
