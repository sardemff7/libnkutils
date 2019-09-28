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

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif /* G_LOG_DOMAIN */
#define G_LOG_DOMAIN "libnkutils-enum"

#include <string.h>

#include <glib.h>

#include "nkutils-enum.h"

static inline gint
nk_str_equal(NkEnumMatchFlags flags, const gchar *token, const gchar *string)
{
    if ( ( token == NULL ) || ( string == NULL ) || ( token == string ) )
        return ( token == string );

    gunichar wt = g_utf8_get_char(token), ws = g_utf8_get_char(string);
    while ( ( wt != '\0' ) && ( ws != '\0' ) )
    {
        if ( flags & NK_ENUM_MATCH_FLAGS_IGNORE_CASE )
        {
            wt = g_unichar_tolower(wt);
            ws = g_unichar_tolower(ws);
        }

        if ( wt != ws )
            return FALSE;

        token = g_utf8_next_char(token);
        string = g_utf8_next_char(string);
        wt = g_utf8_get_char(token);
        ws = g_utf8_get_char(string);
    }
    if ( flags & NK_ENUM_MATCH_FLAGS_PREFIX_STRING )
        return ( wt == '\0' );
    if ( flags & NK_ENUM_MATCH_FLAGS_PREFIX_VALUE )
        return ( ws == '\0' );
    return ( ws == wt );
}

gboolean
nk_enum_parse(const gchar *string, const gchar * const *values, guint64 size, NkEnumMatchFlags flags, guint64 *value)
{
    guint64 i;
    for ( i = 0 ; i < size ; ++i )
    {
        if ( nk_str_equal(flags, values[i], string) )
        {
            *value = i;
            return TRUE;
        }
    }
    return FALSE;
}
