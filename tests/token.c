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
#include <nkutils-glib-compat.h>

#include <nkutils-token.h>

#define MAX_DATA 4

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
    {
        .testpath = "/nkutils/token/basic/before-after/with",
        .data = {
            .source = "You can make a ${(<adjective>) }${recipe} with ${fruit}${ and <addition}.",
            .data = {
                "adjective", "creamy",
                "fruit", "a banana",
                "recipe", "banana split",
                "addition", "some cream",
                NULL
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/basic/before-after/without",
        .data = {
            .source = "You can make a ${(<adjective>) }${recipe} with ${fruit}${ and <addition}.",
            .data = {
                "fruit", "a banana",
                "recipe", "banana split",
                NULL
            },
            .result = "You can make a banana split with a banana."
        }
    },
};

static const gchar *
_nk_token_list_tests_callback(const gchar *token, guint64 value, gconstpointer user_data)
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

#ifdef NK_ENABLE_TOKEN_ENUM
typedef enum {
    TOKEN_FRUIT,
    TOKEN_RECIPE,
    _TOKEN_SIZE
} NkTokenListEnumTokens;

static const gchar const * const _nk_token_list_enum_tests_tokens[_TOKEN_SIZE] = {
    [TOKEN_FRUIT]  = "fruit",
    [TOKEN_RECIPE] = "recipe",
};

typedef struct {
    const gchar *source;
    const gchar * const data[_TOKEN_SIZE];
    const gchar *result;
} NkTokenListEnumTestData;

static const struct {
    const gchar *testpath;
    NkTokenListEnumTestData data;
} _nk_token_list_enum_tests_list[] = {
    {
        .testpath = "/nkutils/token/enum/basic",
        .data = {
            .source = "You can make ${recipe} with ${fruit}.",
            .data = {
                [TOKEN_FRUIT]  = "a banana",
                [TOKEN_RECIPE] = "a banana split",
            },
            .result = "You can make a banana split with a banana."
        }
    },
};

static const gchar *
_nk_token_list_enum_tests_callback(const gchar *token, guint64 value, gconstpointer user_data)
{
    const gchar * const *data = user_data;
    return data[value];
}

static void
_nk_token_list_enum_tests_func(gconstpointer user_data)
{
    const NkTokenListEnumTestData *data = user_data;

    NkTokenList *token_list;
    token_list = nk_token_list_parse_enum(g_strdup(data->source), _nk_token_list_enum_tests_tokens, _TOKEN_SIZE, NULL);
    g_assert_nonnull(token_list);

    gchar *result;
    result = nk_token_list_replace(token_list, _nk_token_list_enum_tests_callback, data->data);

    g_assert_cmpstr(result, ==, data->result);

    nk_token_list_unref(token_list);
}
#endif /* NK_ENABLE_TOKEN_ENUM */

int
main(int argc, char *argv[])
{
    g_type_init();

    g_test_init(&argc, &argv, NULL);

    g_test_set_nonfatal_assertions();

    gsize i;
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_token_list_tests_list) ; ++i )
        g_test_add_data_func(_nk_token_list_tests_list[i].testpath, &_nk_token_list_tests_list[i].data, _nk_token_list_tests_func);

#ifdef NK_ENABLE_TOKEN_ENUM
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_token_list_enum_tests_list) ; ++i )
        g_test_add_data_func(_nk_token_list_enum_tests_list[i].testpath, &_nk_token_list_enum_tests_list[i].data, _nk_token_list_enum_tests_func);
#endif /* NK_ENABLE_TOKEN_ENUM */

    return g_test_run();
}
