/*
 * libnkutils/token - Miscellaneous utilities, token module
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

#include <string.h>

#include <glib.h>

#include <nkutils-token.h>

#define MAX_DATA 4

typedef struct {
    const gchar *token;
    const gchar *key;
    gint64 index;
    const gchar *content;
} NkTokenTestDataData;

typedef struct {
    const gchar *source;
    NkTokenTestDataData data[MAX_DATA + 1];
    gint error;
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
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/dash-underscore",
        .data = {
            .source = "You can make ${recipe_name} with ${fruit-name}.",
            .data = {
                { .token = "fruit-name", .content = "a banana" },
                { .token = "recipe_name", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/unicode",
        .data = {
            .source = "You can make ${recette} with ${ingrédient}.",
            .data = {
                { .token = "ingrédient", .content = "a banana" },
                { .token = "recette", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/basic/wrong/1",
        .data = {
            .source = "You can make ${recipe} with $fruit.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with $fruit."
        }
    },
    {
        .testpath = "/nkutils/token/basic/wrong/2",
        .data = {
            .source = "$fruit is good.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "$fruit is good."
        }
    },
    {
        .testpath = "/nkutils/token/key/index",
        .data = {
            .source = "You can make a ${recipe[0]} with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .key = "", .index = 0, .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/key/name",
        .data = {
            .source = "You can make a ${recipe[icecream]} with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .key = "icecream", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/key/name/modifier",
        .data = {
            .source = "You can make a ${recipe[cake]:-banana cake} with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .key = "cream", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana cake with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/key/join/replace",
        .data = {
            .source = "You can make [${recipes[@@]/@/], [}] with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipes", .key = "@@", .content = "banana pie@banana split" },
                { .token = NULL }
            },
            .result = "You can make [banana pie], [banana split] with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/wrong/modifier",
        .data = {
            .source = "You can make a ${recipe} with ${fruit::}.",
            .error = NK_TOKEN_ERROR_UNKNOWN_MODIFIER,
        }
    },
    {
        .testpath = "/nkutils/token/wrong/key/index",
        .data = {
            .source = "You can make a ${recipe[18446744073709551616]} with ${fruit}.",
            .error = NK_TOKEN_ERROR_WRONG_KEY,
        }
    },
    {
        .testpath = "/nkutils/token/wrong/key",
        .data = {
            .source = "You can make a ${recipe[|]} with ${fruit}.",
            .error = NK_TOKEN_ERROR_WRONG_KEY,
        }
    },
    {
        .testpath = "/nkutils/token/wrong/regex/pattern",
        .data = {
            .source = "You can make a ${recipe/[} with ${fruit}.",
            .error = NK_TOKEN_ERROR_REGEX,
        }
    },
    {
        .testpath = "/nkutils/token/wrong/regex/replace",
        .data = {
            .source = "You can make ${recipe/a/\\gwrong} with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "You can make  with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/fallback/with",
        .data = {
            .source = "I want to eat ${fruit:-an apple}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = NULL }
            },
            .result = "I want to eat a banana."
        }
    },
    {
        .testpath = "/nkutils/token/fallback/without",
        .data = {
            .source = "I want to eat ${fruit:-an apple}.",
            .data = {
                { .token = NULL }
            },
            .result = "I want to eat an apple."
        }
    },
    {
        .testpath = "/nkutils/token/fallback/recurse",
        .data = {
            .source = "I want to eat ${fruit:-${vegetable}}.",
            .data = {
                { .token = "vegetable", .content = "a zucchini" },
                { .token = NULL }
            },
            .result = "I want to eat a zucchini."
        }
    },
    {
        .testpath = "/nkutils/token/substitute/with",
        .data = {
            .source = "You can make a ${adjective:+(}${adjective}${adjective:+) }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                { .token = "adjective", .content = "creamy" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/substitute/without",
        .data = {
            .source = "You can make a ${adjective:+(}${adjective}${adjective:+) }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/anti-substitute/with",
        .data = {
            .source = "I want to eat a ${adjective:!sweat }lemon.",
            .data = {
                { .token = "adjective", .content = "juicy" },
                { .token = NULL }
            },
            .result = "I want to eat a lemon."
        }
    },
    {
        .testpath = "/nkutils/token/anti-substitute/without",
        .data = {
            .source = "I want to eat a ${adjective:!sweat }lemon.",
            .data = {
                { .token = NULL }
            },
            .result = "I want to eat a sweat lemon."
        }
    },
    {
        .testpath = "/nkutils/token/replace/full",
        .data = {
            .source = "You can make a ${recipe/split/cream} with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana cream with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/replace/missing",
        .data = {
            .source = "You can make a ${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/replace/capture",
        .data = {
            .source = "You can make a ${adjective/(.+)/(\\1) }${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .token = "adjective", .content = "creamy" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/replace/before-after/with",
        .data = {
            .source = "You can make a ${adjective/^/(/$/) }${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .token = "adjective", .content = "creamy" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/replace/before-after/without",
        .data = {
            .source = "You can make a ${adjective/^/(/$/) }${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/replace/remove",
        .data = {
            .source = "You can make a ${recipe/ split} with ${fruit}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a banana with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/replace/multiple",
        .data = {
            .source = "You can make ${recipe/a banana/an apple pie/ split} with ${fruit/.+/apples}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "a banana split" },
                { .token = NULL }
            },
            .result = "You can make an apple pie with apples."
        }
    },
    {
        .testpath = "/nkutils/token/replace/braces/paired",
        .data = {
            .source = "You can make a ${adjective/.{2}$/y/^/(/$/) }${recipe} with ${fruit}${addition/\\{//\\}//^/ and }.",
            .data = {
                { .token = "adjective", .content = "creamed" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream{}" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/replace/braces/opening",
        .data = {
            .source = "You can make a ${adjective/.{2}$/y/^/(/$/) }${recipe} with ${fruit}${addition/\\{//^/ and }.",
            .data = {
                { .token = "adjective", .content = "creamed" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream{" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/replace/braces/closing",
        .data = {
            .source = "You can make a ${adjective/.{2}$/y/^/(/$/) }${recipe} with ${fruit}${addition/\\}//^/ and }.",
            .data = {
                { .token = "adjective", .content = "creamed" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream}" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/replace/escaping/backslash",
        .data = {
            .source = "You can make a ${adjective/^/(/$/) /\\\\}${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .token = "adjective", .content = "creamy\\" },
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = "addition", .content = "some cream" },
                { .token = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/token/replace/escaping/forwardslash/1",
        .data = {
            .source = "${data/\\/}",
            .data = {
                { .token = "data", .content = "/" },
                { .token = NULL }
            },
            .result = ""
        }
    },
    {
        .testpath = "/nkutils/token/replace/escaping/forwardslash/2",
        .data = {
            .source = "${data/a/\\/}",
            .data = {
                { .token = "data", .content = "a" },
                { .token = NULL }
            },
            .result = "/"
        }
    },
    {
        .testpath = "/nkutils/token/replace/escaping/forwardslash/3",
        .data = {
            .source = "${data/a/\\//b/x}",
            .data = {
                { .token = "data", .content = "ab" },
                { .token = NULL }
            },
            .result = "/x"
        }
    },
    {
        .testpath = "/nkutils/token/replace/escaping/right-curly-bracket/2",
        .data = {
            .source = "${data/a/\\}}",
            .data = {
                { .token = "data", .content = "a" },
                { .token = NULL }
            },
            .result = "}"
        }
    },
    {
        .testpath = "/nkutils/token/replace/recurse/with",
        .data = {
            .source = "I want to eat ${recipe/an apple/${fruit}}.",
            .data = {
                { .token = "recipe", .content = "an apple pie" },
                { .token = "fruit", .content = "a blackberry" },
                { .token = NULL }
            },
            .result = "I want to eat a blackberry pie."
        }
    },
    {
        .testpath = "/nkutils/token/old/before-after",
        .data = {
            .source = "You can make a ${(<adjective>) }${recipe} with ${fruit}${ and <addition}.",
            .data = {
                { .token = "fruit", .content = "a banana" },
                { .token = "recipe", .content = "banana split" },
                { .token = NULL }
            },
            .result = "You can make a ${(<adjective>) }banana split with a banana${ and <addition}."
        }
    },
    {
        .testpath = "/nkutils/token/double-dollar",
        .data = {
            .source = "echo $${PATH}",
            .data = {
                { .token = NULL }
            },
            .result = "echo ${PATH}"
        }
    },
};

static const gchar *
_nk_token_list_tests_callback(const gchar *token, guint64 value, const gchar *key, gint64 index, gpointer user_data)
{
    NkTokenTestData *test_data = user_data;
    NkTokenTestDataData *data;
    g_assert_cmpuint(value, ==, 0);
    for ( data = test_data->data ; data->token != NULL ; ++data )
    {
        if ( ( g_strcmp0(token, data->token) == 0 ) && ( g_strcmp0(key, data->key) == 0 ) && ( index == data->index ) )
            return data->content;
    }
    return NULL;
}

static void
_nk_token_list_tests_func(gconstpointer user_data)
{
    NkTokenTestData *data = (NkTokenTestData *) user_data;
    NkTokenList *token_list;
    GError *error = NULL;

    token_list = nk_token_list_parse(g_strdup(data->source), &error);
    if ( data->result == NULL )
    {
        g_assert_null(token_list);
        g_assert_error(error, NK_TOKEN_ERROR, data->error);
        return;
    }
    g_assert_nonnull(token_list);
    g_assert_no_error(error);
    g_assert_nonnull(nk_token_list_ref(token_list));

    gchar *result;
    result = nk_token_list_replace(token_list, _nk_token_list_tests_callback, data);

    g_assert_cmpstr(result, ==, data->result);

    nk_token_list_unref(token_list);
    nk_token_list_unref(token_list);
}

typedef enum {
    TOKEN_FRUIT,
    TOKEN_RECIPE,
    _TOKEN_SIZE
} NkTokenListEnumTokens;

static const gchar * const _nk_token_list_enum_tests_tokens[_TOKEN_SIZE] = {
    [TOKEN_FRUIT]  = "fruit",
    [TOKEN_RECIPE] = "recipe",
};

typedef struct {
    const gchar *source;
    gchar *data[_TOKEN_SIZE];
    guint64 used_tokens;
    gint error;
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
            .used_tokens = (1 << TOKEN_FRUIT) | (1 << TOKEN_RECIPE),
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/token/enum/wrong/regex",
        .data = {
            .source = "You can make a ${recipe/[} with ${fruit}.",
            .error = NK_TOKEN_ERROR_REGEX,
        }
    },
    {
        .testpath = "/nkutils/token/enum/wrong/token",
        .data = {
            .source = "You can make a ${recipe} with ${fruit} and ${addition}.",
            .error = NK_TOKEN_ERROR_UNKNOWN_TOKEN,
        }
    },
};

static const gchar *
_nk_token_list_enum_tests_callback(const gchar *token, guint64 value, G_GNUC_UNUSED const gchar *key, G_GNUC_UNUSED  gint64 index, gpointer user_data)
{
    const gchar * const *data = user_data;
    g_assert_cmpstr(token, ==, _nk_token_list_enum_tests_tokens[value]);
    return data[value];
}

static void
_nk_token_list_enum_tests_func(gconstpointer user_data)
{
    NkTokenListEnumTestData *data = (NkTokenListEnumTestData *) user_data;
    NkTokenList *token_list;
    guint64 used_tokens;
    GError *error = NULL;

    token_list = nk_token_list_parse_enum(g_strdup(data->source), _nk_token_list_enum_tests_tokens, _TOKEN_SIZE, &used_tokens, &error);
    if ( data->result == NULL )
    {
        g_assert_null(token_list);
        g_assert_error(error, NK_TOKEN_ERROR, data->error);
        return;
    }
    g_assert_no_error(error);
    g_assert_nonnull(token_list);
    if ( data->used_tokens != 0 )
        g_assert_cmpuint(used_tokens, ==, data->used_tokens);

    gchar *result;
    result = nk_token_list_replace(token_list, _nk_token_list_enum_tests_callback, data->data);

    g_assert_cmpstr(result, ==, data->result);

    nk_token_list_unref(token_list);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_set_nonfatal_assertions();

    gsize i;
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_token_list_tests_list) ; ++i )
        g_test_add_data_func(_nk_token_list_tests_list[i].testpath, &_nk_token_list_tests_list[i].data, _nk_token_list_tests_func);

    for ( i = 0 ; i < G_N_ELEMENTS(_nk_token_list_enum_tests_list) ; ++i )
        g_test_add_data_func(_nk_token_list_enum_tests_list[i].testpath, &_nk_token_list_enum_tests_list[i].data, _nk_token_list_enum_tests_func);

    return g_test_run();
}
