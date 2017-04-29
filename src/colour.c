/*
 * libnkutils/colour - Miscellaneous utilities, colour module
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

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif /* G_LOG_DOMAIN */
#define G_LOG_DOMAIN "libnkutils-colour"

#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "nkutils-colour.h"


static GScanner *_nk_colour_scanner = NULL;

typedef enum {
    NK_COLOUR_SCOPE_BASE,
} NkColourScope;

typedef enum {
    NK_COLOUR_SYMBOL_BASE_RGB,
    NK_COLOUR_SYMBOL_BASE_RGBA,
} NkColourSymbolBase;

static const gchar * const _nk_colour_scanner_symbols_base[] = {
    [NK_COLOUR_SYMBOL_BASE_RGB] = "rgb",
    [NK_COLOUR_SYMBOL_BASE_RGBA] = "rgba",
};

#define IS_BETWEEN(x, low, high) ( ( (x) >= (low) ) && ( (x) <= (high) ) )

static inline gboolean
_nk_colour_parse_hex(gdouble *r, gchar c1, gchar c2)
{
    gchar s[3] = { c1, c2, '\0' }, *e;
    guint64 v;

    v = g_ascii_strtoull(s, &e, 16);
    if ( s == e )
        return FALSE;
    if ( v > 255 )
        return FALSE;

    *r = (gdouble) v / 255.;
    return TRUE;
}

static inline gboolean
_nk_colour_parse_alpha_value(gdouble *r, gboolean expected)
{
    if ( expected != ( g_scanner_peek_next_token(_nk_colour_scanner) == G_TOKEN_COMMA ) )
        return FALSE;
    if ( ! expected )
        return TRUE;
    g_scanner_get_next_token(_nk_colour_scanner);

    if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_FLOAT )
        return FALSE;

    gdouble v = _nk_colour_scanner->value.v_float;
    if ( g_scanner_peek_next_token(_nk_colour_scanner) == '%' )
    {
        g_scanner_get_next_token(_nk_colour_scanner);

        if ( ! IS_BETWEEN(v, 0., 100.) )
            return FALSE;

        *r = v / 100.;
    }
    else
    {
        if ( ! IS_BETWEEN(v, 0., 1.) )
            return FALSE;

        *r = v;
    }

    return TRUE;
}

static inline gboolean
_nk_colour_parse_rgb_value(gdouble *r)
{
    if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_FLOAT )
        return FALSE;

    gdouble v = _nk_colour_scanner->value.v_float;
    if ( g_scanner_peek_next_token(_nk_colour_scanner) == '%' )
    {
        g_scanner_get_next_token(_nk_colour_scanner);

        if ( ! IS_BETWEEN(v, 0., 100.) )
            return FALSE;

        *r = v / 100.;
    }
    else
    {
        if ( ! IS_BETWEEN(v, 0., 255.) )
            return FALSE;

        *r = v / 255.;
    }

    return TRUE;
}

gboolean
nk_colour_parse(const gchar *s, NkColour *colour)
{
    if ( _nk_colour_scanner == NULL )
    {
        _nk_colour_scanner = g_scanner_new(NULL);
        _nk_colour_scanner->config->int_2_float = TRUE;
        _nk_colour_scanner->config->cset_identifier_first = "#" G_CSET_a_2_z G_CSET_A_2_Z G_CSET_LATINS G_CSET_LATINC;
        _nk_colour_scanner->config->cpair_comment_single = "";
        _nk_colour_scanner->config->case_sensitive = TRUE;

        gsize i;
        for ( i = 0 ; i < G_N_ELEMENTS(_nk_colour_scanner_symbols_base) ; ++i )
            g_scanner_scope_add_symbol(_nk_colour_scanner, NK_COLOUR_SCOPE_BASE, _nk_colour_scanner_symbols_base[i], GUINT_TO_POINTER(i));
    }

    if ( s == NULL )
        return FALSE;

    g_scanner_input_text(_nk_colour_scanner, s, strlen(s));
    g_scanner_set_scope(_nk_colour_scanner, NK_COLOUR_SCOPE_BASE);

    NkColour colour_ = { .alpha = 1. };
    switch ( g_scanner_get_next_token(_nk_colour_scanner) )
    {
    case G_TOKEN_SYMBOL:
    {
        NkColourSymbolBase symbol = _nk_colour_scanner->value.v_int;
        gboolean alpha = FALSE;
        switch ( symbol )
        {
        case NK_COLOUR_SYMBOL_BASE_RGBA:
            alpha = TRUE;
        case NK_COLOUR_SYMBOL_BASE_RGB:
            if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_LEFT_PAREN )
                return FALSE;
            if ( ! _nk_colour_parse_rgb_value(&colour_.red) )
                return FALSE;
            if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_COMMA )
                return FALSE;
            if ( ! _nk_colour_parse_rgb_value(&colour_.green) )
                return FALSE;
            if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_COMMA )
                return FALSE;
            if ( ! _nk_colour_parse_rgb_value(&colour_.blue) )
                return FALSE;
            if ( ! _nk_colour_parse_alpha_value(&colour_.alpha, alpha) )
                return FALSE;
            if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_RIGHT_PAREN )
                return FALSE;
        break;
        }
    }
    break;
    case G_TOKEN_IDENTIFIER:
        if ( _nk_colour_scanner->value.v_identifier[0] == '#' )
        {
            if ( g_scanner_peek_next_token(_nk_colour_scanner) != G_TOKEN_EOF )
                return FALSE;

            const gchar *hex = _nk_colour_scanner->value.v_identifier + strlen("#");
            switch ( strlen(hex) )
            {
            case 8: /* rrggbbaa */
                if ( ! _nk_colour_parse_hex(&colour_.alpha, hex[6], hex[7]) )
                    return FALSE;
                /* fallthrough */
            case 6: /* rrggbb */
                if ( ! _nk_colour_parse_hex(&colour_.red, hex[0], hex[1]) )
                    return FALSE;
                if ( ! _nk_colour_parse_hex(&colour_.green, hex[2], hex[3]) )
                    return FALSE;
                if ( ! _nk_colour_parse_hex(&colour_.blue, hex[4], hex[5]) )
                    return FALSE;
            break;
            case 4: /* rgba */
                if ( ! _nk_colour_parse_hex(&colour_.alpha, hex[3], hex[3]) )
                    return FALSE;
                /* fallthrough */
            case 3: /* rgb */
                if ( ! _nk_colour_parse_hex(&colour_.red, hex[0], hex[0]) )
                    return FALSE;
                if ( ! _nk_colour_parse_hex(&colour_.green, hex[1], hex[1]) )
                    return FALSE;
                if ( ! _nk_colour_parse_hex(&colour_.blue, hex[2], hex[2]) )
                    return FALSE;
            break;
            default:
                return FALSE;
            }
        }
        else
            return FALSE;
    break;
    default:
        return FALSE;
    }

    if ( g_scanner_get_next_token(_nk_colour_scanner) != G_TOKEN_EOF )
        return FALSE;

    *colour = colour_;

    return TRUE;
}

#define HEX_COLOUR_MAXLEN 10 /* strlen("#rrggbbaa") + 1 */
const gchar *
nk_colour_to_hex(const NkColour *colour)
{
    guint8 red   = (guint8) ( colour->red   * 255. + 0.5 );
    guint8 green = (guint8) ( colour->green * 255. + 0.5 );
    guint8 blue  = (guint8) ( colour->blue  * 255. + 0.5 );
    guint8 alpha = (guint8) ( colour->alpha * 255. + 0.5 );

    static gchar string[HEX_COLOUR_MAXLEN];
    if ( alpha != 0xff )
        g_snprintf(string, HEX_COLOUR_MAXLEN, "#%02x%02x%02x%02x", red, green, blue, alpha);
    else
        g_snprintf(string, HEX_COLOUR_MAXLEN, "#%02x%02x%02x", red, green, blue);
    return string;
}

#define COLOUR_DOUBLE_RGBA_MAXLEN 64 /* strlen("rgba(255.0000000000,255.0000000000,255.0000000000,0.0000000000)") + 1 */
const gchar *
nk_colour_to_rgba(const NkColour *colour)
{
    gdouble red   = colour->red   * 255.;
    gdouble green = colour->green * 255.;
    gdouble blue  = colour->blue  * 255.;
    gdouble alpha = colour->alpha;

    static gchar string[COLOUR_DOUBLE_RGBA_MAXLEN];
    if ( alpha != 1.0 )
        g_snprintf(string, COLOUR_DOUBLE_RGBA_MAXLEN, "rgba(%.10lf,%.10lf,%.10lf,%.10lf)", red, green, blue, alpha);
    else
        g_snprintf(string, COLOUR_DOUBLE_RGBA_MAXLEN, "rgb(%.10lf,%.10lf,%.10lf)", red, green, blue);
    return string;
}

