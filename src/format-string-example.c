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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif /* G_LOG_DOMAIN */
#define G_LOG_DOMAIN "nk-format-string-replace"

#include <string.h>
#include <locale.h>
#include <errno.h>

#include <glib.h>

#include "nkutils-format-string.h"

typedef struct {
    int argc;
    gchar **argv;
} Args;

static GVariant *
_nk_format_string_replace_reference_callback(const gchar *name, G_GNUC_UNUSED guint64 value, gpointer user_data)
{
    Args *args = user_data;
    int i;
    for ( i = 0 ; i < args->argc ; ++i )
    {
        const gchar *arg = args->argv[i];
        if ( ! g_str_has_prefix(arg, name) )
            continue;
        arg += strlen(name);
        switch ( g_utf8_get_char(arg) )
        {
        case '\0':
            if ( ++i < args->argc )
                return g_variant_parse(NULL, args->argv[i], NULL, NULL, NULL);
        break;
        case '=':
            return g_variant_parse(NULL, g_utf8_next_char(arg), NULL, NULL, NULL);
        default:
        break;
        }
    }
    return NULL;
}

int
main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    if ( argc < 2 )
    {
        g_warning("You must provide a format string");
        return 1;
    }

    NkFormatString *template;
    Args args = {
        .argc = argc - 2,
        .argv = argv + 2,
    };
    gchar *result;
    GError *error = NULL;

    template = nk_format_string_parse(g_strdup(argv[1]), '$', &error);
    if ( template == NULL )
    {
        g_warning("Template string could not be parsed: %s", error->message);
        g_clear_error(&error);
        return 2;
    }
    result = nk_format_string_replace(template, _nk_format_string_replace_reference_callback, &args);

    g_print("%s\n", result);

    g_free(result);
    nk_format_string_unref(template);

    return 0;
}
