/*
 * libnkutils/format-string - Miscellaneous utilities, format string module
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

#ifndef __NK_UTILS_FORMAT_STRING_H__
#define __NK_UTILS_FORMAT_STRING_H__


typedef enum {
    NK_FORMAT_STRING_ERROR_WRONG_KEY,
    NK_FORMAT_STRING_ERROR_UNKNOWN_MODIFIER,
    NK_FORMAT_STRING_ERROR_WRONG_RANGE,
    NK_FORMAT_STRING_ERROR_WRONG_SWITCH,
    NK_FORMAT_STRING_ERROR_WRONG_PRETIFFY,
    NK_FORMAT_STRING_ERROR_REGEX,
    NK_FORMAT_STRING_ERROR_UNKNOWN_TOKEN,
} NkFormatStringError;

typedef struct _NkFormatString NkFormatString;

typedef GVariant *(*NkFormatStringReplaceReferenceCallback)(const gchar *name, guint64 value, gpointer user_data);

GQuark nk_format_string_error_quark(void);
#define NK_FORMAT_STRING_ERROR (nk_format_string_error_quark())

NkFormatString *nk_format_string_parse(gchar *string, gunichar identifier, GError **error);
NkFormatString *nk_format_string_parse_enum(gchar *string, gunichar identifier, const gchar * const *tokens, guint64 size, guint64 *used_tokens, GError **error);
NkFormatString *nk_format_string_ref(NkFormatString *format_string);
void nk_format_string_unref(NkFormatString *format_string);
gchar *nk_format_string_replace(const NkFormatString *format_string, NkFormatStringReplaceReferenceCallback callback, gpointer user_data);

#endif /* __NK_UTILS_FORMAT_STRING_H__ */
