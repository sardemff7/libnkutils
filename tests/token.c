/*
 * libnkutils/token - Miscellaneous utilities, token module
 *
 * Copyright © 2011-2015 Quentin "Sardem FF7" Glidic
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

#include <string.h>

#include <glib.h>

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
        .testpath = "/nkutils/token/basic/wrong",
        .data = {
            .source = "You can make ${recipe} with $fruit.",
            .data = {
                "fruit", "a banana",
                "recipe", "a banana split",
                NULL
            },
            .result = "You can make a banana split with $fruit."
        }
    },
    {
        .testpath = "/nkutils/token/basic/wrong/0",
        .data = {
            .source = "$fruit is good.",
            .data = {
                "fruit", "a banana",
                "recipe", "a banana split",
                NULL
            },
            .result = "$fruit is good."
        }
    },
    {
        .testpath = "/nkutils/token/basic/wrong/modifier",
        .data = {
            .source = "You can make a ${recipe} with a ${fruit::}.",
            .data = {
                "fruit", "a banana",
                "recipe", "banana split",
                NULL
            },
            .result = "You can make a banana split with a ${fruit::}."
        }
    },
    {
        .testpath = "/nkutils/token/basic/fallback/with",
        .data = {
            .source = "I want to eat ${fruit:-an apple}.",
            .data = {
                "fruit", "a banana",
                NULL
            },
            .result = "I want to eat a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/fallback/without",
        .data = {
            .source = "I want to eat ${fruit:-an apple}.",
            .data = {
                NULL
            },
            .result = "I want to eat an apple."
        }
    },
    {
        .testpath = "/nkutils/token/basic/substitute/with",
        .data = {
            .source = "You can make a ${adjective:+(}${adjective}${adjective:+) }${recipe} with ${fruit}${addition:+ and }${addition}.",
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
        .testpath = "/nkutils/token/basic/substitute/without",
        .data = {
            .source = "You can make a ${adjective:+(}${adjective}${adjective:+) }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                "fruit", "a banana",
                "recipe", "banana split",
                NULL
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/replace/full",
        .data = {
            .source = "You can make a ${recipe/split/cream} with ${fruit}.",
            .data = {
                "fruit", "a banana",
                "recipe", "banana split",
                NULL
            },
            .result = "You can make a banana cream with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/replace/capture",
        .data = {
            .source = "You can make a ${adjective/(.+)/(\\1)}${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                "fruit", "a banana",
                "recipe", "banana split",
                NULL
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/replace/remove",
        .data = {
            .source = "You can make a ${recipe/ split} with ${fruit}.",
            .data = {
                "fruit", "a banana",
                "recipe", "banana split",
                NULL
            },
            .result = "You can make a banana with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/replace/multiple",
        .data = {
            .source = "You can make ${recipe/a banana/an apple pie/ split} with ${fruit/.+/apples}.",
            .data = {
                "fruit", "a banana",
                "recipe", "a banana split",
                NULL
            },
            .result = "You can make an apple pie with apples."
        }
    },
};

static const gchar *
_nk_token_list_tests_callback(const gchar *token, guint64 value, gconstpointer user_data)
{
    const gchar * const *data;
    g_assert_cmpuint(value, ==, 0);
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
    g_assert_cmpstr(token, ==, _nk_token_list_enum_tests_tokens[value]);
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
