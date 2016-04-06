/*
 * libnkutils/token - Miscellaneous utilities, token module
 *
 * Copyright © 2011-2016 Quentin "Sardem FF7" Glidic
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
    GRegex *regex;
    const gchar *replacement;
} NkTokenRegex;

typedef struct {
    const gchar *string;
    const gchar *name;
    guint64 value;
    const gchar *fallback;
    const gchar *substitute;
    NkTokenRegex *replace;
} NkToken;

struct _NkTokenList {
    guint64 ref_count;
    gchar *string;
    gsize size;
    NkToken *tokens;
};

static gchar *
_nk_token_strchr_escape(gchar *s, gssize l, gunichar c, gunichar pair_c)
{
    if ( l < 0 )
        l = strlen(s);
    gchar *e = s + l;

    gsize pair_count = 0;
    gchar *w = s;
    gunichar wc = '\0', pc;

    for ( ; w < e ; w = g_utf8_next_char(w) )
    {
        pc = wc;
        wc = g_utf8_get_char(w);

        if ( pc == '\\' )
        {
            /* Escaped, search for next one */
            if ( wc == '\\' )
                /* Escaping a backslash, avoid escaping the next char */
                wc = '\0';
            continue;
        }

        /* Maybe do we open a paired character */
        if ( ( pair_c != '\0' ) && ( wc == pair_c ) )
            ++pair_count;

        if ( wc != c )
            continue;

        /* We found our character, check if it is the right occurence */

        if ( ( pair_c != '\0' ) && ( pair_count > 0 ) )
        {
            /* We had an opened pair, close it */
            --pair_count;
            continue;
        }

        return w;
    }
    return NULL;
}

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
        gchar *b = w;
        w = g_utf8_next_char(w);
        switch ( g_utf8_get_char(w) )
        {
        case '{':
        {
            w = g_utf8_next_char(w);
            gchar *e;
            NkToken token = {
                .name = w
            };

            /* References are ASCII only */
            while ( g_ascii_isalpha(*w) || ( *w == '-' ) || ( *w == '_' ) )
                w = g_utf8_next_char(w);


            e = _nk_token_strchr_escape(w, -1, '}', '{');
            if ( e == NULL )
                continue;


            if ( w != e )
            switch ( g_utf8_get_char(w) )
            {
            case ':':
            {
                gchar *m = w++;
                switch ( g_utf8_get_char(w) )
                {
                case '-':
                    token.fallback = ++w;
                break;
                case '+':
                    token.substitute = ++w;
                break;
                default:
                    /* Just fail on malformed string */
                    continue;
                }
                *m = '\0';
            }
            break;
            case '/':
            {
                gchar *m = w;
                *e = '\0';

                gsize c = 0;
                do
                {
                    ++c;
                    *m = '\0';
                } while ( ( m = _nk_token_strchr_escape(m, e - m, '/', '\0') ) != NULL );
                c = ( c + 1 ) / 2 + 1;

                token.replace = g_new(NkTokenRegex, c);
                c = 0;
                do
                {
                    token.replace[c].regex = g_regex_new(++w, G_REGEX_OPTIMIZE, 0, NULL);
                    if ( token.replace[c].regex == NULL )
                    {
                        /* Malformed regex, we revert all changes made to the string */
                        *b = '$';
                        *e = '}';
                        for ( m = b ; m < e ; ++m )
                        {
                            if ( *m == '\0' )
                                *m = '/';
                        }
                        w = e;
                        continue;
                    }

                    w = w + strlen(w) + 1;
                    token.replace[c].replacement = ( w > e ) ? "" : w;
                    w += strlen(w);
                    ++c;
                } while ( w < e );
                token.replace[c].regex = NULL;
            }
            break;
            default:
                continue;
            }

            *e = *b = '\0';

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

            string = w = e + 1;
            break;
        }
        case '$':
            ++w;
        default:
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
        if ( self->tokens[i].replace != NULL )
        {
            NkTokenRegex *regex;
            for ( regex = self->tokens[i].replace ; regex->regex != NULL ; ++regex )
            {
                if ( regex->regex != NULL )
                    g_regex_unref(regex->regex);
            }
            g_free(self->tokens[i].replace);
        }
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
            else if ( self->tokens[i].replace != NULL )
            {
                NkTokenRegex *regex;
                gchar *n = NULL;
                for ( regex = self->tokens[i].replace ; regex->regex != NULL ; ++regex )
                {
                    gchar *tmp = n;
                    n = g_regex_replace(regex->regex, data, -1, 0, regex->replacement, 0, NULL);
                    g_free(tmp);
                    if ( n == NULL )
                        break;
                    data = n;
                }
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
