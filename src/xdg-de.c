/*
 * libnkutils/xdg-de - Miscellaneous utilities, XDG DE module
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
#define G_LOG_DOMAIN "libnkutils-de"

#include <glib.h>

#include "nkutils-enum.h"
#include "nkutils-xdg-de.h"

static const gchar * const _nk_xdg_de_session_desktop_names[] = {
    [NK_DE_NONE] = "generic",
    [NK_DE_GNOME] = "GNOME",
    [NK_DE_KDE] = "KDE",
};

static const gchar * const _nk_xdg_de_current_session_names[] = {
    [NK_DE_NONE] = "X-Generic",
    [NK_DE_GNOME] = "GNOME",
    [NK_DE_KDE] = "KDE",
};

static const gchar * const _nk_xdg_de_desktop_session_names[] = {
    [NK_DE_NONE] = "generic",
    [NK_DE_GNOME] = "gnome",
    [NK_DE_KDE] = "kde",
};

NkXdgDE
nk_xdg_de_detect(void)
{
    static NkXdgDE _nk_xdg_de = NK_DE_NONE - 1;
    if ( _nk_xdg_de != (NkXdgDE) ( NK_DE_NONE - 1 ) )
        return _nk_xdg_de;

    const gchar *var;
    guint64 value;

    var = g_getenv("XDG_SESSION_DESKTOP");
    if ( ( var != NULL ) && nk_enum_parse(var, _nk_xdg_de_session_desktop_names, G_N_ELEMENTS(_nk_xdg_de_session_desktop_names), TRUE, TRUE, &value) )
    {
        _nk_xdg_de = value;
        return _nk_xdg_de;
    }

    var = g_getenv("XDG_CURRENT_DESKTOP");
    if ( ( var != NULL ) && nk_enum_parse(var, _nk_xdg_de_current_session_names, G_N_ELEMENTS(_nk_xdg_de_current_session_names), FALSE, TRUE, &value) )
    {
        _nk_xdg_de = value;
        return _nk_xdg_de;
    }

    var = g_getenv("GNOME_DESKTOP_SESSION_ID");
    if ( ( var != NULL ) && ( *var != '\0' ) )
    {
        _nk_xdg_de = NK_DE_GNOME;
        return _nk_xdg_de;
    }

    var = g_getenv("KDE_FULL_SESSION");
    if ( ( var != NULL ) && ( *var != '\0' ) )
    {
        _nk_xdg_de = NK_DE_KDE;
        return _nk_xdg_de;
    }

    var = g_getenv("DESKTOP_SESSION");
    if ( ( var != NULL ) && nk_enum_parse(var, _nk_xdg_de_desktop_session_names, G_N_ELEMENTS(_nk_xdg_de_desktop_session_names), FALSE, FALSE, &value) )
    {
        _nk_xdg_de = value;
        return _nk_xdg_de;
    }

    _nk_xdg_de = NK_DE_NONE;
    return _nk_xdg_de;
}
