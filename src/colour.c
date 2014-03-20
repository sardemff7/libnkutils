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
#ifdef NK_ENABLE_COLOUR_STRING
#include <glib/gprintf.h>
#endif /* NK_ENABLE_COLOUR_STRING */

#include "nkutils-colour.h"


#define _nk_colour_parse_hex(r, c1, c2) G_STMT_START { \
    gchar s__[3] = { c1, c2, '\0' }; \
    gchar *e__; \
    guint64 v__; \
    v__ = g_ascii_strtoull(s__, &e__, 16); \
    if ( s__ == e__ ) return FALSE; \
    if ( v__ > 255 ) return FALSE; \
    r = v__; \
    } G_STMT_END

#define _nk_colour_walk_white(s) G_STMT_START { while ( g_ascii_isspace(*s) ) ++s; } G_STMT_END
#define _nk_colour_check_number_full(s, r, t, f, v1, v2, ...) G_STMT_START { \
    _nk_colour_walk_white(s); \
    gchar *e__; \
    t v__; \
    v__ = f(s, &e__, ## __VA_ARGS__); \
    if ( s == e__ ) return FALSE; \
    s = e__; \
    r = CLAMP(v__, v1, v2); \
    _nk_colour_walk_white(s); \
    } G_STMT_END
#define _nk_colour_check_number(s, r) G_STMT_START { \
    _nk_colour_check_number_full(s, r, gint64, g_ascii_strtoll, 0, 255, 10); \
    if ( *s == '%' ) { r = ( MIN(r, 100) * 255 ) / 100; ++s; } \
    _nk_colour_walk_white(s); \
    } G_STMT_END
#define _nk_colour_check_comma(s) G_STMT_START { if ( *s != ',' ) return FALSE; ++s; } G_STMT_END

#define _nk_colour_uint8_to_double(u) (((gdouble)(u)) / 255.)

static gboolean
_nk_colour_parse(const gchar *s, NkColour *colour, gdouble *ra)
{
    if ( s == NULL )
        return FALSE;

    guint8 r = 0, g = 0, b = 0, a = 255;
    gdouble da = 1.;

    if ( g_str_has_prefix(s, "#") )
    {
        s += strlen("#");
        switch ( strlen(s) )
        {
        case 8: /* rrggbbaa */
            _nk_colour_parse_hex(a, s[6], s[7]);
            da = _nk_colour_uint8_to_double(a);
        case 6: /* rrggbb */
            _nk_colour_parse_hex(r, s[0], s[1]);
            _nk_colour_parse_hex(g, s[2], s[3]);
            _nk_colour_parse_hex(b, s[4], s[5]);
        break;
        case 4: /* rgba */
            _nk_colour_parse_hex(a, s[3], s[3]);
            da = _nk_colour_uint8_to_double(a);
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

#ifdef NK_ENABLE_COLOUR_ALPHA
        gboolean alpha;
        alpha = ( *s == 'a' );
        if ( alpha ) ++s;
#endif /* NK_ENABLE_COLOUR_ALPHA */

        if ( *s++ != '(' )
            return FALSE;

        _nk_colour_check_number(s, r);
        _nk_colour_check_comma(s);
        _nk_colour_check_number(s, g);
        _nk_colour_check_comma(s);
        _nk_colour_check_number(s, b);
#ifdef NK_ENABLE_COLOUR_ALPHA
        if ( alpha )
        {
            _nk_colour_check_comma(s);
            _nk_colour_check_number_full(s, da, gdouble, g_ascii_strtod, 0., 1.);
            a = da * 255;
        }
#endif /* NK_ENABLE_COLOUR_ALPHA */
        if ( g_strcmp0(s, ")") != 0 ) return FALSE;
    }
    else
        return FALSE;

    colour->red   = r;
    colour->green = g;
    colour->blue  = b;
    colour->alpha = a;
    if ( ra != NULL )
        *ra = da;

    return TRUE;
}

gboolean
nk_colour_parse(const gchar *string, NkColour *colour)
{
    return _nk_colour_parse(string, colour, NULL);
}

#ifdef NK_ENABLE_COLOUR_DOUBLE
gboolean
nk_colour_double_parse(const gchar *string, NkColourDouble *colour)
{
    NkColour colour_;
    gdouble a;

    if ( _nk_colour_parse(string, &colour_, &a) )
    {
        colour->red   = _nk_colour_uint8_to_double(colour_.red);
        colour->green = _nk_colour_uint8_to_double(colour_.green);
        colour->blue  = _nk_colour_uint8_to_double(colour_.blue);
        colour->alpha = a;

        return TRUE;
    }

    return FALSE;
}
#endif /* NK_ENABLE_COLOUR_DOUBLE */

#ifdef NK_ENABLE_COLOUR_STRING

const gchar *
nk_colour_to_hex(const NkColour *colour)
{
    static gchar string[10]; /* strlen("#rrggbbaa") + 1 */
#ifdef NK_ENABLE_COLOUR_ALPHA
    if ( colour->alpha != 0xff )
        g_sprintf(string, "#%02x%02x%02x%02x", colour->red, colour->green, colour->blue, colour->alpha);
    else
#endif /* NK_ENABLE_COLOUR_ALPHA */
        g_sprintf(string, "#%02x%02x%02x", colour->red, colour->green, colour->blue);
    return string;
}

static inline void
_nk_colour_to_rgba_internal(gchar *string, guint8 red, guint8 green, guint8 blue, gint alpha_precision, gdouble alpha)
{
#ifdef NK_ENABLE_COLOUR_ALPHA
    if ( alpha != 1.0 )
        g_sprintf(string, "rgba(%u,%u,%u,%.*lf)", red, green, blue, alpha_precision, alpha);
    else
#endif /* NK_ENABLE_COLOUR_ALPHA */
        g_sprintf(string, "rgb(%u,%u,%u)", red, green, blue);
}

const gchar *
nk_colour_to_rgba(const NkColour *colour)
{
    static gchar string[24]; /* strlen("rgba(255,255,255,0.000)") + 1 */
    _nk_colour_to_rgba_internal(string, colour->red, colour->green, colour->blue, 3, (gdouble)colour->alpha / 255.);
    return string;
}

#ifdef NK_ENABLE_COLOUR_DOUBLE
const gchar *
nk_colour_double_to_hex(const NkColourDouble *colour)
{
    NkColour colour_ = {
        .red   = colour->red   * 255,
        .green = colour->green * 255,
        .blue  = colour->blue  * 255,
        .alpha = colour->alpha * 255
    };
    return nk_colour_to_hex(&colour_);
}

const gchar *
nk_colour_double_to_rgba(const NkColourDouble *colour)
{
    static gchar string[31]; /* strlen("rgba(255,255,255,0.0000000000)") + 1 */
    _nk_colour_to_rgba_internal(string, colour->red * 255, colour->green * 255, colour->blue * 255, 10, colour->alpha);
    return string;
}
#endif /* NK_ENABLE_COLOUR_DOUBLE */

#endif /* NK_ENABLE_COLOUR_STRING */
