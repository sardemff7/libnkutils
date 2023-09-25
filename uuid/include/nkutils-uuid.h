/*
 * libnkutils/uuid - Miscellaneous utilities, uuid module
 *
 * Copyright © 2011-2021 Quentin "Sardem FF7" Glidic
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

#ifndef __NK_UTILS_UUID_H__
#define __NK_UTILS_UUID_H__

#define NK_UUID_LENGTH 16
#define NK_UUID_FORMATTED_LENGTH 36

typedef struct {
    guchar data[NK_UUID_LENGTH];
    gchar string[NK_UUID_FORMATTED_LENGTH + 1];
} NkUuid;

#define NK_UUID_INIT { .data = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, .string = "00000000-0000-0000-0000-000000000000" }
#define NK_UUID_MAKE(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) { .data = { 0x##a, 0x##b, 0x##c, 0x##d, 0x##e, 0x##f, 0x##g, 0x##h, 0x##i, 0x##j, 0x##k, 0x##l, 0x##m, 0x##n, 0x##o, 0x##p }, .string = #a #b #c #d "-" #e #f "-" #g #h "-" #i #j "-" #k #l #m #n #o #p }
#define NK_UUID_IS_NULL(uuid) ((uuid)->data[0] == 0 && (uuid)->data[1] == 0 && (uuid)->data[2] == 0 && (uuid)->data[3] == 0 && (uuid)->data[4] == 0 && (uuid)->data[5] == 0 && (uuid)->data[6] == 0 && (uuid)->data[7] == 0 && (uuid)->data[8] == 0 && (uuid)->data[9] == 0 && (uuid)->data[10] == 0 && (uuid)->data[11] == 0 && (uuid)->data[12] == 0 && (uuid)->data[13] == 0 && (uuid)->data[14] == 0 && (uuid)->data[15] == 0)

void nk_uuid_generate(NkUuid *uuid);
gboolean nk_uuid_parse(NkUuid *uuid, const gchar *string);

void nk_uuid_from_name(NkUuid *uuid, const gchar *name, gssize length);

void nk_uuid_get_machine_app_specific(NkUuid *uuid, NkUuid app_uuid);

#endif /* __NK_UTILS_UUID_H__ */
