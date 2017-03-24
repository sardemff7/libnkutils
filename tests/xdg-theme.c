/*
 * libnkutils/token - Miscellaneous utilities, token module
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

#include <string.h>

#include <glib.h>

#include <nkutils-xdg-theme.h>

static NkXdgThemeContext *context;
typedef enum {
    TYPE_NONE = 0,
    TYPE_ICON,
    TYPE_SOUND,
} NkXdgThemeTestType;

typedef struct {
    gboolean no_skip;
    NkXdgThemeTestType type;
    const gchar *theme;
    const gchar *name;
    gint size;
    gboolean scalable;
    const gchar *profile;
    const gchar *result;
} NkXdgThemeTestData;

static const struct {
    const gchar *testpath;
    NkXdgThemeTestData data;
} _nk_uuid_tests_list[] = {
    {
        .testpath = "/nkutils/xdg-theme/icon/symbolic/found",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "zoom-in-symbolic",
            .size = 48,
            .scalable = TRUE,
            .result = "/usr/share/icons/gnome/scalable/actions/zoom-in-symbolic.svg"
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/symbolic/not-scalable",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "zoom-in-symbolic",
            .size = 48,
            .scalable = FALSE,
            .result = "/usr/share/icons/gnome/48x48/actions/zoom-in.png"
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/symbolic/found-no-symbolic",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "trophy-gold-symbolic",
            .size = 48,
            .scalable = TRUE,
            .result = "/usr/share/icons/gnome/48x48/status/trophy-gold.png"
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/not-found",
        .data = {
            .no_skip = TRUE,
            .type = TYPE_ICON,
            .theme = "do-not-exists-hopefully",
            .name = "nothing-on-earth-will-have-that-name",
            .size = 48,
            .scalable = TRUE,
            .result = NULL
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme",
        .data = {
            .no_skip = TRUE,
            .type = TYPE_ICON,
            .theme = NULL,
            .name = "nothing-on-earth-will-have-that-name",
            .size = 48,
            .scalable = TRUE,
            .result = NULL
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/sound/fallback/freedesktop",
        .data = {
            .type = TYPE_SOUND,
            .theme = "non-existing-i-hope",
            .name = "dialog-information",
            .profile = "stereo",
            .result = "/usr/share/sounds/freedesktop/stereo/dialog-information.oga"
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/sound/fallback/no-index",
        .data = {
            .type = TYPE_SOUND,
            .theme = "purple",
            .name = "logout",
            .profile = "stereo",
            .result = "/usr/share/sounds/purple/logout.wav"
        }
    },
};

static void
_nk_uuid_tests_func(gconstpointer user_data)
{
    const NkXdgThemeTestData *data = user_data;

    if ( ( ! data->no_skip) && ( ! g_file_test(data->result, G_FILE_TEST_IS_REGULAR) ) )
    {
        g_test_skip("Theme not installed");
        return;
    }

    gchar *file;
    switch ( data->type )
    {
    case TYPE_ICON:
        file = nk_xdg_theme_get_icon(context, data->theme, data->name, data->size, data->scalable);
    break;
    case TYPE_SOUND:
        file = nk_xdg_theme_get_sound(context, data->theme, data->name, data->profile, NULL);
    break;
    default:
        g_assert_not_reached();
    }
    g_assert_cmpstr(file, ==, data->result);
    g_free(file);
}

#include <locale.h>
int
main(int argc, char *argv[])
{
  setlocale (LC_ALL, "");
    g_test_init(&argc, &argv, NULL);

    g_test_set_nonfatal_assertions();

    g_setenv("HOME", "/var/empty", TRUE);
    g_setenv("XDG_DATA_HOME", "/var/empty", TRUE);
    g_setenv("XDG_DATA_DIRS", "/usr/share/", TRUE);

    gsize i;
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_uuid_tests_list) ; ++i )
        g_test_add_data_func(_nk_uuid_tests_list[i].testpath, &_nk_uuid_tests_list[i].data, _nk_uuid_tests_func);

    context = nk_xdg_theme_context_new();
    int ret = g_test_run();
    nk_xdg_theme_context_free(context);
    return ret;
}
