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

#define NK_COMMA ,

#define _nk_colour_parse_hex(r, c1, c2) G_STMT_START { \
    gchar s__[3] = { c1, c2, '\0' }; \
    gchar *e__; \
    guint64 v__; \
    v__ = g_ascii_strtoull(s__, &e__, 16); \
    if ( s__ == e__ ) return FALSE; \
    if ( v__ > 255 ) return FALSE; \
    r = (gdouble) v__ / 255.; \
    } G_STMT_END

#define _nk_colour_walk_white(s) G_STMT_START { while ( g_ascii_isspace(*s) ) ++s; } G_STMT_END
#define _nk_colour_check_number_full(s, r, c) G_STMT_START { \
    _nk_colour_walk_white(s); \
    gchar *e__; \
    gdouble v__; \
    v__ = g_ascii_strtod(s, &e__); \
    if ( s == e__ ) return FALSE; \
    s = e__; \
    if ( *s == '%' ) { r = CLAMP(v__, 0., 100.) / 100.; ++s; } \
    else { r = c } \
    _nk_colour_walk_white(s); \
    } G_STMT_END
#define _nk_colour_check_number(s, r) _nk_colour_check_number_full(s, r, CLAMP(v__, 0., 255.) / 255.; )
#define _nk_colour_check_comma(s) G_STMT_START { if ( *s != ',' ) return FALSE; ++s; } G_STMT_END

#define _nk_colour_uint8_to_double(u) (((gdouble)(u)) / 255.)

static gboolean
_nk_colour_parse(const gchar *s, NkColour *colour)
{
    if ( s == NULL )
        return FALSE;

    gdouble r = 0., g = 0., b = 0., a = 1.;

    if ( g_str_has_prefix(s, "#") )
    {
        s += strlen("#");
        switch ( strlen(s) )
        {
        case 8: /* rrggbbaa */
            _nk_colour_parse_hex(a, s[6], s[7]);
            /* fallthrough */
        case 6: /* rrggbb */
            _nk_colour_parse_hex(r, s[0], s[1]);
            _nk_colour_parse_hex(g, s[2], s[3]);
            _nk_colour_parse_hex(b, s[4], s[5]);
        break;
        case 4: /* rgba */
            _nk_colour_parse_hex(a, s[3], s[3]);
            /* fallthrough */
        case 3: /* rgb */
            _nk_colour_parse_hex(r, s[0], s[0]);
            _nk_colour_parse_hex(g, s[1], s[1]);
            _nk_colour_parse_hex(b, s[2], s[2]);
        break;
        default:
            return FALSE;
        }
    }
    else if ( g_str_has_prefix(s, "rgb") )
    {

        s += strlen("rgb");

        gboolean alpha;
        alpha = ( *s == 'a' );
        if ( alpha ) ++s;

        if ( *s++ != '(' )
            return FALSE;

        _nk_colour_check_number(s, r);
        _nk_colour_check_comma(s);
        _nk_colour_check_number(s, g);
        _nk_colour_check_comma(s);
        _nk_colour_check_number(s, b);
        if ( alpha )
        {
            _nk_colour_check_comma(s);
            _nk_colour_check_number_full(s, a, CLAMP(v__, 0., 1.););
        }
        if ( g_strcmp0(s, ")") != 0 )
            return FALSE;
    }
    else
        return FALSE;

    colour->red   = r;
    colour->green = g;
    colour->blue  = b;
    colour->alpha = a;

    return TRUE;
}

gboolean
nk_colour_parse(const gchar *string, NkColour *colour)
{
    NkColour colour_;

    if ( _nk_colour_parse(string, &colour_) )
    {
        *colour = colour_;
        return TRUE;
    }

    return FALSE;
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

