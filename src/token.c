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
#include <nkutils-glib-compat.h>

#ifdef NK_ENABLE_TOKEN_ENUM
#include "nkutils-enum.h"
#endif /* NK_ENABLE_TOKEN_ENUM */

#include "nkutils-token.h"

typedef struct {
    const gchar *string;
    const gchar *name;
    guint64 value;
    const gchar *before;
    const gchar *after;
} NkToken;

struct _NkTokenList {
    guint64 ref_count;
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
    self->ref_count = 1;
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
                ++self->size;
                if ( *string != '\0' )
                    ++self->size;
                self->tokens = g_renew(NkToken, self->tokens, self->size);
                if ( *string != '\0' )
                {
                    self->tokens[self->size - 2].string = string;
                    self->tokens[self->size - 2].before = NULL;
                    self->tokens[self->size - 2].name = NULL;
                    self->tokens[self->size - 2].after = NULL;
                }
                w = string = e + 1;

                const gchar *a = NULL, *b = NULL;
                gchar *m;
                m = g_utf8_strchr(n, -1, '<');
                if ( m != NULL )
                {
                    *m = '\0';
                    b = n;
                    n = ++m;
                }
                m = g_utf8_strchr(n, -1, '>');
                if ( m != NULL )
                {
                    *m = '\0';
                    a = ++m;
                }
                self->tokens[self->size - 1].string = NULL;
                self->tokens[self->size - 1].before = b;
                self->tokens[self->size - 1].name = n;
                self->tokens[self->size - 1].after = a;
                break;
            }
        case '$':
            ++n;
        default:
            w = ++n;
        break;
        }
    }
    self->tokens = g_renew(NkToken, self->tokens, ++self->size);
    self->tokens[self->size - 1].string = string;
    self->tokens[self->size - 1].before = NULL;
    self->tokens[self->size - 1].name = NULL;
    self->tokens[self->size - 1].after = NULL;

    return self;
}

#ifdef NK_ENABLE_TOKEN_ENUM
NkTokenList *
nk_token_list_parse_enum(gchar *string, const gchar * const *tokens, guint64 size, guint64 *ret_used_tokens)
{
    g_return_val_if_fail(string != NULL, NULL);

    NkTokenList *self;
    guint64 used_tokens = 0;

    self = nk_token_list_parse(string);

    gsize i;
    for ( i = 0 ; i < self->size ; ++i )
    {
        if ( self->tokens[i].name == NULL )
            continue;
        if ( ! nk_enum_parse(self->tokens[i].name, tokens, size, FALSE, &self->tokens[i].value) )
            goto fail;
        used_tokens |= (1 << self->tokens[i].value);
    }

    if ( ret_used_tokens != NULL )
        *ret_used_tokens = used_tokens;

    return self;

fail:
    nk_token_list_unref(self);
    return NULL;
}
#endif /* NK_ENABLE_TOKEN_ENUM */

NkTokenList *
nk_token_list_ref(NkTokenList *self)
{
    g_return_val_if_fail(self != NULL, NULL);
    ++self->ref_count;
    return self;
}

void
nk_token_list_unref(NkTokenList *self)
{
    g_return_if_fail(self != NULL);
    if ( --self->ref_count > 0 )
        return;

    g_free(self->tokens);

    g_free(self->string);

    g_free(self);
}

gchar *
nk_token_list_replace(const NkTokenList *self, NkTokenListReplaceCallback callback, gconstpointer user_data)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(callback != NULL, NULL);

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

        const gchar *data;
        data = callback(self->tokens[i].name, self->tokens[i].value, user_data);
        if ( data != NULL )
        {
            if ( self->tokens[i].before != NULL)
                g_string_append(string, self->tokens[i].before);
            g_string_append(string, data);
            if ( self->tokens[i].after != NULL)
                g_string_append(string, self->tokens[i].after);
        }
    }

    return g_string_free(string, FALSE);
}
