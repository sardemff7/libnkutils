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

#ifndef __NK_UTILS_TOKEN_H__
#define __NK_UTILS_TOKEN_H__

typedef struct _NkTokenList NkTokenList;

typedef gchar *(*NkTokenListReplaceCallback)(const gchar *token, gconstpointer user_data);

NkTokenList *nk_token_list_parse(gchar *string);
NkTokenList *nk_token_list_ref(NkTokenList *token_list);
void nk_token_list_unref(NkTokenList *token_list);
gchar *nk_token_list_replace(const NkTokenList *token_list, NkTokenListReplaceCallback callback, gconstpointer user_data);

#endif /* __NK_UTILS_TOKEN_H__ */
