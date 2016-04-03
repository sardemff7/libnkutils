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

#ifndef __NK_UTILS_TOKEN_H__
#define __NK_UTILS_TOKEN_H__

typedef struct _NkTokenList NkTokenList;

typedef const gchar *(*NkTokenListReplaceCallback)(const gchar *token, guint64 value, gconstpointer user_data);

NkTokenList *nk_token_list_parse(gchar *string);
NkTokenList *nk_token_list_parse_enum(gchar *string, const gchar * const *tokens, guint64 size, guint64 *used_tokens);
NkTokenList *nk_token_list_ref(NkTokenList *token_list);
void nk_token_list_unref(NkTokenList *token_list);
gchar *nk_token_list_replace(const NkTokenList *token_list, NkTokenListReplaceCallback callback, gconstpointer user_data);

#endif /* __NK_UTILS_TOKEN_H__ */
