/*
 * libnkutils/enum - Miscellaneous utilities, enum module
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib.h>

#include <nkutils-enum.h>

typedef enum {
    CENTER,
    LEFT,
    RIGHT,
    _MAX_VALUES
} NkEnumTestValues;

static const gchar * const _nk_enum_tests_values[_MAX_VALUES] = {
    [CENTER] = "center",
    [LEFT] = "left",
    [RIGHT] = "right",
};

typedef struct {
    const gchar *string;
    gboolean ignore_case;
    gboolean ret;
    NkEnumTestValues value;
    const gchar * const values[_MAX_VALUES];
} NkEnumTestData;

static const struct {
    const gchar *testpath;
    NkEnumTestData data;
} _nk_enum_tests_list[] = {
    {
        .testpath = "/nkutils/enum/exists",
        .data = {
            .string = "center",
            .ignore_case = FALSE,
            .ret = TRUE,
            .value = CENTER,
        }
    },
    {
        .testpath = "/nkutils/enum/missing",
        .data = {
            .string = "Center",
            .ignore_case = FALSE,
            .ret = FALSE,
            .value = _MAX_VALUES,
        }
    },
    {
        .testpath = "/nkutils/enum/case",
        .data = {
            .string = "Center",
            .ignore_case = TRUE,
            .ret = TRUE,
            .value = CENTER,
        }
    },
};

static void
_nk_enum_tests_func(gconstpointer user_data)
{
    const NkEnumTestData *data = user_data;

    guint64 value = _MAX_VALUES;
    gboolean r;
    r = nk_enum_parse(data->string, _nk_enum_tests_values, _MAX_VALUES, data->ignore_case, &value);
    g_assert(data->ret == r);
    g_assert_cmpuint(data->value, ==, value);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_set_nonfatal_assertions();

    gsize i;
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_enum_tests_list) ; ++i )
        g_test_add_data_func(_nk_enum_tests_list[i].testpath, &_nk_enum_tests_list[i].data, _nk_enum_tests_func);

    return g_test_run();
}
