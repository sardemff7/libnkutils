/*
 * libnkutils/enum - Miscellaneous utilities, enum module
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
