/*
 * libnkutils - Small daemon to act on remote or local events
 *
 * Copyright © 2011-2014 Quentin "Sardem FF7" Glidic
 *
 * This file is part of libnkutils.
 *
 * libnkutils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libnkutils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libnkutils. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __NKUTILS_GLIB_COMPAT_H__
#define __NKUTILS_GLIB_COMPAT_H__

#if ! GLIB_CHECK_VERSION(2, 31, 0)
#define g_test_undefined() (FALSE)
#endif

#if GLIB_CHECK_VERSION(2,35,1)
#define g_type_init() {}
#endif /* GLIB_CHECK_VERSION(2,35,1) */


#if ! GLIB_CHECK_VERSION(2, 38, 0)
#define g_test_set_nonfatal_assertions() {}
#define g_assert_true(expr) g_assert(expr)

#define g_assert_null(expr) g_assert_true((expr) == NULL)
#endif

#if ! GLIB_CHECK_VERSION(2, 40, 0)
#define g_assert_nonnull(expr) g_assert_true((expr) != NULL)
#endif

#endif /* __NKUTILS_GLIB_COMPAT_H__ */
