/*
 * libnkutils/token - Miscellaneous utilities, token module
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

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif /* G_LOG_DOMAIN */
#define G_LOG_DOMAIN "libnkutils-token"

#include <string.h>

#include <glib.h>

#ifdef NK_ENABLE_TOKEN_ENUM
#include "nkutils-enum.h"
#endif /* NK_ENABLE_TOKEN_ENUM */

#include "nkutils-token.h"

typedef struct {
    const gchar *string;
    const gchar *name;
    guint64 value;
    const gchar *fallback;
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
                    self->tokens[self->size - 2].name = NULL;
                    self->tokens[self->size - 2].fallback = NULL;
                    self->tokens[self->size - 2].before = NULL;
                    self->tokens[self->size - 2].after = NULL;
                }
                w = string = e + 1;

                const gchar *a = NULL, *b = NULL, *d = NULL;
                gchar *m;
                m = g_utf8_strchr(n, -1, ':');
                if ( m != NULL )
                {
                    *m = '\0';
                    d = ++m;
                }
                else
                {
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
                }
                self->tokens[self->size - 1].string = NULL;
                self->tokens[self->size - 1].name = n;
                self->tokens[self->size - 1].fallback = d;
                self->tokens[self->size - 1].before = b;
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
    self->tokens[self->size - 1].name = NULL;
    self->tokens[self->size - 1].fallback = NULL;
    self->tokens[self->size - 1].before = NULL;
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
        else if ( self->tokens[i].fallback != NULL )
            g_string_append(string, self->tokens[i].fallback);
    }

    return g_string_free(string, FALSE);
}
