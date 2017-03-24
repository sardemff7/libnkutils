/*
 * libnkutils/enum - Miscellaneous utilities, enum module
 *
 * Copyright © 2011-2017 Quentin "Sardem FF7" Glidic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
