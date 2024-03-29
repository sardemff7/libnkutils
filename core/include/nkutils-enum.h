/*
 * libnkutils/enum - Miscellaneous utilities, enum module
 *
 * Copyright © 2011-2024 Morgane "Sardem FF7" Glidic
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

#ifndef __NK_UTILS_ENUM_H__
#define __NK_UTILS_ENUM_H__

typedef enum {
    NK_ENUM_MATCH_FLAGS_NONE          = 0,
    NK_ENUM_MATCH_FLAGS_IGNORE_CASE   = (1 << 1),
    NK_ENUM_MATCH_FLAGS_PREFIX_VALUE  = (1 << 2),
    NK_ENUM_MATCH_FLAGS_PREFIX_STRING = (1 << 3),
} NkEnumMatchFlags;

gboolean nk_enum_parse(const gchar *string, const gchar * const *values, guint64 size, NkEnumMatchFlags flags, guint64 *value);

#endif /* __NK_UTILS_ENUM_H__ */
