/*
 * libnkutils/enum - Miscellaneous utilities, enum module
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

#include "nkutils-enum.h"

gboolean
nk_enum_parse(const gchar *string, const gchar * const *values, guint64 size, gboolean ignore_case, guint64 *value)
{
    gint (*string_compare)(const gchar *, const gchar *);
    if ( ignore_case )
        string_compare = g_ascii_strcasecmp;
    else
        string_compare = g_strcmp0;
    guint64 i;
    for ( i = 0 ; i < size ; ++i )
    {
        if ( string_compare(string, values[i]) == 0 )
        {
            *value = i;
            return TRUE;
        }
    }
    return FALSE;
}
