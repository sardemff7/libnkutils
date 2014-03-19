/*
 * libnkutils/colour - Miscellaneous utilities, colour module
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

#ifndef __NK_UTILS_COLOUR_H__
#define __NK_UTILS_COLOUR_H__

typedef struct {
    guint8 red;
    guint8 green;
    guint8 blue;
    guint8 alpha;
} NkColour;

typedef struct {
    gdouble red;
    gdouble green;
    gdouble blue;
    gdouble alpha;
} NkColourDouble;

gboolean nk_colour_parse(const gchar *string, NkColour *colour);
gboolean nk_colour_double_parse(const gchar *string, NkColourDouble *colour);
const gchar *nk_colour_to_hex(const NkColour *colour);
const gchar *nk_colour_to_rgba(const NkColour *colour);
const gchar *nk_colour_double_to_hex(const NkColourDouble *colour);
const gchar *nk_colour_double_to_rgba(const NkColourDouble *colour);

#endif /* __NK_UTILS_COLOUR_H__ */
