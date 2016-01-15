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
    const gchar *substitute;
    struct {
        GRegex *regex;
        const gchar *replacement;
    } replace;
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

    gchar *w = string;
    while ( ( w = g_utf8_strchr(w, -1, '$') ) != NULL )
    {
        switch ( w[1] )
        {
        case '{':
        {
            gchar *b = w, *n = w + 2, *e;
            e = g_utf8_strchr(n, -2, '}');
            if ( e != NULL )
            {
                w = e + 1;

                NkToken token = {
                    .name = n
                };

                gchar *m;
                if ( ( m = g_utf8_strchr(n, e - n, ':') ) != NULL )
                {
                    switch ( m[1] )
                    {
                    case '-':
                        token.fallback = m + 2;
                    break;
                    case '+':
                        token.substitute = m + 2;
                    break;
                    default:
                        /* We will treat the malformed reference as a string */
                        continue;
                    }
                    *m = '\0';
                }
                else if ( ( m = g_utf8_strchr(n, e - n, '/') ) != NULL )
                {
                    gchar *s = m + 1;

                    while ( ( s = g_utf8_strchr(s, s - n, '/') ) != NULL )
                    {
                        if ( *(s - 1) != '\\' )
                            break;
                    }

                    gsize l = ( ( s != NULL ) ? s : e ) - m;
                    gchar r[l];
                    strncpy(r, m + 1, l);
                    r[l-1] = 0;

                    token.replace.regex = g_regex_new(r, G_REGEX_OPTIMIZE, 0, NULL);
                    token.replace.replacement = ( s != NULL ) ? ( s + 1 ) : "";
                    if ( token.replace.regex == NULL )
                        /* We will treat the malformed reference as a string */
                        continue;
                    *m = '\0';
                }

                ++self->size;
                if ( *string != '\0' )
                    ++self->size;
                self->tokens = g_renew(NkToken, self->tokens, self->size);
                if ( *string != '\0' )
                {
                    NkToken stoken = {
                        .string = string
                    };
                    self->tokens[self->size - 2] = stoken;
                }
                self->tokens[self->size - 1] = token;

                *e = *b = '\0';
                string = w;
                break;
            }
        }
        case '$':
            ++w;
        default:
            ++w;
        break;
        }
    }
    self->tokens = g_renew(NkToken, self->tokens, ++self->size);
    NkToken token = {
        .string = string
    };
    self->tokens[self->size - 1] = token;

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

    gsize i;
    for ( i = 0 ; i < self->size ; ++i )
    {
        if ( self->tokens[i].replace.regex != NULL )
            g_regex_unref(self->tokens[i].replace.regex);
    }

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
            if ( self->tokens[i].substitute != NULL)
                g_string_append(string, self->tokens[i].substitute);
            else if ( self->tokens[i].replace.regex != NULL )
            {
                gchar *n;
                n = g_regex_replace(self->tokens[i].replace.regex, data, -1, 0, self->tokens[i].replace.replacement, 0, NULL);
                if ( n != NULL )
                    g_string_append(string, n);
                g_free(n);
            }
            else
                g_string_append(string, data);
        }
        else if ( self->tokens[i].fallback != NULL )
            g_string_append(string, self->tokens[i].fallback);
    }

    return g_string_free(string, FALSE);
}
