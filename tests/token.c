/*
 * libnkutils/token - Miscellaneous utilities, token module
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include <glib.h>

#include <nkutils-token.h>

#define MAX_DATA 2

typedef struct {
    const gchar *source;
    const gchar *data[MAX_DATA * 2 + 1];
    const gchar *result;
} NkTokenTestData;

static const struct {
    const gchar *testpath;
    NkTokenTestData data;
} _nk_token_list_tests_list[] = {
    {
        .testpath = "/nkutils/token/basic",
        .data = {
            .source = "You can make ${recipe} with ${fruit}.",
            .data = {
                "fruit", "a banana",
                "recipe", "a banana split",
                NULL
            },
            .result = "You can make a banana split with a banana."
        }
    },
};

static const gchar *
_nk_token_list_tests_callback(const gchar *token, gconstpointer user_data)
{
    const gchar * const *data;
    for ( data = user_data ; *data != NULL ; data += 2 )
    {
        if ( g_strcmp0(token, *data) == 0 )
            return *++data;
    }
    return NULL;
}

static void
_nk_token_list_tests_func(gconstpointer user_data)
{
    const NkTokenTestData *data = user_data;

    NkTokenList *token_list;
    token_list = nk_token_list_parse(g_strdup(data->source));
    g_assert_nonnull(token_list);

    gchar *result;
    result = nk_token_list_replace(token_list, _nk_token_list_tests_callback, data->data);

    g_assert_cmpstr(result, ==, data->result);

    nk_token_list_unref(token_list);
}

int
main(int argc, char *argv[])
{
#if ! GLIB_CHECK_VERSION(2,35,1)
    g_type_init();
#endif /* ! GLIB_CHECK_VERSION(2,35,1) */

    g_test_init(&argc, &argv, NULL);

#if GLIB_CHECK_VERSION(2,38,0)
    g_test_set_nonfatal_assertions();
#endif /* GLIB_CHECK_VERSION(2,38,0) */

    gsize i;
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_token_list_tests_list) ; ++i )
        g_test_add_data_func(_nk_token_list_tests_list[i].testpath, &_nk_token_list_tests_list[i].data, _nk_token_list_tests_func);

    return g_test_run();
}
