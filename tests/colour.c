/*
 * libnkutils/colour - Miscellaneous utilities, colour module
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

#include <nkutils-colour.h>

typedef struct {
    const gchar *string;
    const gchar *generated_string;
    const NkColour colour;
} NkColourTestData;

static const struct {
    const gchar *testpath;
    NkColourTestData data;
} _nk_colour_tests_list[] = {
#ifdef NK_ENABLE_COLOUR_ALPHA
    {
        .testpath = "/nkutils/colour/hex/8",
        .data ={
            .string = "#ffddee7f",
            .colour = {
                .red   = 0xff,
                .green = 0xdd,
                .blue  = 0xee,
                .alpha = 0x7f
            }
        }
    },
#endif /* NK_ENABLE_COLOUR_ALPHA */
    {
        .testpath = "/nkutils/colour/hex/6",
        .data ={
            .string = "#121314",
            .colour = {
                .red   = 0x12,
                .green = 0x13,
                .blue  = 0x14,
                .alpha = 0xff
            }
        }
    },
#ifdef NK_ENABLE_COLOUR_ALPHA
    {
        .testpath = "/nkutils/colour/hex/4",
        .data ={
            .string = "#abcd",
            .generated_string = "#aabbccdd",
            .colour = {
                .red   = 0xaa,
                .green = 0xbb,
                .blue  = 0xcc,
                .alpha = 0xdd
            }
        }
    },
#endif /* NK_ENABLE_COLOUR_ALPHA */
    {
        .testpath = "/nkutils/colour/hex/3",
        .data ={
            .string = "#369",
            .generated_string = "#336699",
            .colour = {
                .red   = 0x33,
                .green = 0x66,
                .blue  = 0x99,
                .alpha = 0xff
            }
        }
    },
    {
        .testpath = "/nkutils/colour/rgb",
        .data ={
            .string = "rgb(255, 127, 0)",
            .generated_string = "rgb(255,127,0)",
            .colour = {
                .red   = 255,
                .green = 127,
                .blue  = 0,
                .alpha = 0xff
            }
        }
    },
    {
        .testpath = "/nkutils/colour/rgb/percentage",
        .data ={
            .string = "rgb(100%, 50%, 0%)",
            .generated_string = "rgb(255,127,0)",
            .colour = {
                .red   = 255,
                .green = 127,
                .blue  = 0,
                .alpha = 0xff
            }
        }
    },
#ifdef NK_ENABLE_COLOUR_ALPHA
    {
        .testpath = "/nkutils/colour/rgba",
        .data ={
            .string = "rgba(152, 237, 3, 0.2)",
            .generated_string = "rgba(152,237,3,0.200)",
            .colour = {
                .red   = 152,
                .green = 237,
                .blue  = 3,
                .alpha = 0x33
            }
        }
    },
#endif /* NK_ENABLE_COLOUR_ALPHA */
};

static void
_nk_colour_tests_func(gconstpointer user_data)
{
    const NkColourTestData *data = user_data;

    NkColour colour = {0};
    gboolean r;
    r = nk_colour_parse(data->string, &colour);
    g_assert_true(r);
    g_assert_cmpuint(colour.red, ==, data->colour.red);
    g_assert_cmpuint(colour.green, ==, data->colour.green);
    g_assert_cmpuint(colour.blue, ==, data->colour.blue);
    g_assert_cmpuint(colour.alpha, ==, data->colour.alpha);

#ifdef NK_ENABLE_COLOUR_STRING
    const gchar *string;
    const gchar *wanted_string = ( data->generated_string != NULL ) ? data->generated_string : data->string;

    if ( data->string[0] == '#' )
        string = nk_colour_to_hex(&data->colour);
    else
        string = nk_colour_to_rgba(&data->colour);

    g_assert_nonnull(string);
    g_assert_cmpstr(string, ==, wanted_string);
#endif /* NK_ENABLE_COLOUR_STRING */
}

#ifdef NK_ENABLE_COLOUR_DOUBLE

#define g_assert_cmpfloat_near(a, b, delta) G_STMT_START { g_assert_cmpfloat((b - delta), <, a); g_assert_cmpfloat(a, <, (b + delta)); } G_STMT_END

typedef struct {
    const gchar *string;
    const gchar *generated_string;
    const NkColourDouble colour;
} NkColourDoubleTestData;

static const struct {
    const gchar *testpath;
    NkColourDoubleTestData data;
} _nk_colour_double_tests_list[] = {
#ifdef NK_ENABLE_COLOUR_ALPHA
    {
        .testpath = "/nkutils/colour/double/hex/8",
        .data ={
            .string = "#ffddee7f",
            .generated_string = "#ffdced7e",
            .colour = {
                .red   = 1.,
                .green = .866,
                .blue  = .933,
                .alpha = .498
            }
        }
    },
#endif /* NK_ENABLE_COLOUR_ALPHA */
    {
        .testpath = "/nkutils/colour/double/hex/6",
        .data ={
            .string = "#121314",
            .generated_string = "#111213",
            .colour = {
                .red   = .070,
                .green = .074,
                .blue  = .078,
                .alpha = 1.
            }
        }
    },
#ifdef NK_ENABLE_COLOUR_ALPHA
    {
        .testpath = "/nkutils/colour/double/hex/4",
        .data ={
            .string = "#abcd",
            .generated_string = "#a9baccdc",
            .colour = {
                .red   = .666,
                .green = .733,
                .blue  = .800,
                .alpha = .866
            }
        }
    },
#endif /* NK_ENABLE_COLOUR_ALPHA */
    {
        .testpath = "/nkutils/colour/double/hex/3",
        .data ={
            .string = "#369",
            .generated_string = "#336699",
            .colour = {
                .red   = .200,
                .green = .400,
                .blue  = .600,
                .alpha = 1.
            }
        }
    },
    {
        .testpath = "/nkutils/colour/double/rgb",
        .data ={
            .string = "rgb(51, 102, 153)",
            .generated_string = "rgb(51,102,153)",
            .colour = {
                .red   = .200,
                .green = .400,
                .blue  = .600,
                .alpha = 1.
            }
        }
    },
#ifdef NK_ENABLE_COLOUR_ALPHA
    {
        .testpath = "/nkutils/colour/double/rgba",
        .data ={
            .string = "rgba(255, 0, 127, .1)",
            .generated_string = "rgba(255,0,126,0.1000000000)",
            .colour = {
                .red   = 1.,
                .green = 0.,
                .blue  = .498,
                .alpha = .1
            }
        }
    },
#endif /* NK_ENABLE_COLOUR_ALPHA */
};

static void
_nk_colour_double_tests_func(gconstpointer user_data)
{
    const NkColourDoubleTestData *data = user_data;

    NkColourDouble colour = {0};
    gboolean r;
    r = nk_colour_double_parse(data->string, &colour);
    g_assert_true(r);
    g_assert_cmpfloat_near(colour.red, data->colour.red, 0.001);
    g_assert_cmpfloat_near(colour.green, data->colour.green, 0.001);
    g_assert_cmpfloat_near(colour.blue, data->colour.blue, 0.001);
    g_assert_cmpfloat_near(colour.alpha, data->colour.alpha, 0.001);

#ifdef NK_ENABLE_COLOUR_STRING
    const gchar *string;
    const gchar *wanted_string = ( data->generated_string != NULL ) ? data->generated_string : data->string;

    if ( data->string[0] == '#' )
        string = nk_colour_double_to_hex(&data->colour);
    else
        string = nk_colour_double_to_rgba(&data->colour);

    g_assert_nonnull(string);
    g_assert_cmpstr(string, ==, wanted_string);
#endif /* NK_ENABLE_COLOUR_STRING */
}


#endif /* NK_ENABLE_COLOUR_DOUBLE */

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
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_colour_tests_list) ; ++i )
        g_test_add_data_func(_nk_colour_tests_list[i].testpath, &_nk_colour_tests_list[i].data, _nk_colour_tests_func);

#ifdef NK_ENABLE_COLOUR_DOUBLE
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_colour_double_tests_list) ; ++i )
        g_test_add_data_func(_nk_colour_double_tests_list[i].testpath, &_nk_colour_double_tests_list[i].data, _nk_colour_double_tests_func);
#endif /* NK_ENABLE_COLOUR_DOUBLE */

    return g_test_run();
}
