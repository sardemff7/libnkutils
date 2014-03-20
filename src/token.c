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

#include "nkutils-token.h"

typedef struct {
    const gchar *string;
    const gchar *name;
} NkToken;

struct _NkTokenList {
    gchar *string;
    gsize size;
    NkToken *tokens;
};

NkTokenList *
nk_token_list_parse(gchar *string)
{
    g_return_val_if_fail(string != NULL, NULL);

    NkTokenList *self;

    self = g_new0(NkTokenList, 1);
    self->string = string;

    gchar *w = string, *n, *e;
    while ( ( n = g_utf8_strchr(w, -1, '$') ) != NULL )
    {
        switch ( n[1] )
        {
        case '{':
            e = g_utf8_strchr(n + 2, -2, '}');
            if ( e != NULL )
            {
                *e = *n = '\0';
                n += 2;
                self->size += 2;
                self->tokens = g_renew(NkToken, self->tokens, self->size);
                self->tokens[self->size - 2].string = string;
                self->tokens[self->size - 2].name = NULL;
                w = string = e + 1;
                self->tokens[self->size - 1].string = NULL;
                self->tokens[self->size - 1].name = n;
                break;
            }
        case '$':
            w = n + 2;
        break;
        }
    }
    self->tokens = g_renew(NkToken, self->tokens, ++self->size);
    self->tokens[self->size - 1].string = string;
    self->tokens[self->size - 1].name = NULL;

    return self;
}

void
nk_token_list_free(NkTokenList *self)
{
    g_return_if_fail(self != NULL);

    g_free(self->tokens);

    g_free(self->string);

    g_free(self);
}

gchar *
nk_token_list_replace(const NkTokenList *self, NkTokenListReplaceCallback callback, gconstpointer user_data)
{
    g_return_if_fail(self != NULL);
    g_return_if_fail(callback != NULL);

    GString *string;
    string = g_string_new("");

    gsize i;
    for ( i = 0 ; i < self->size ; ++i )
    {
        if ( self->tokens[i].string != NULL )
        {
            g_string_append(string, self->tokens[i].string);
            continue;
        }

        gchar *data;
        data = callback(self->tokens[i].name, user_data);
        if ( data != NULL )
            g_string_append(string, data);
        g_free(data);
    }

    return g_string_free(string, FALSE);
}
