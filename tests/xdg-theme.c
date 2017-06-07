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
#include <locale.h>

#include <glib.h>

#include <nkutils-xdg-theme.h>

static NkXdgThemeContext *context;
typedef enum {
    TYPE_NONE = 0,
    TYPE_ICON,
    TYPE_SOUND,
} NkXdgThemeTestType;

typedef struct {
    NkXdgThemeTestType type;
    const gchar *theme;
    const gchar *name;
    const gchar *context;
    gint size;
    gint scale;
    gboolean svg;
    const gchar *profile;
    const gchar *result;
    const gchar *theme_test;
} NkXdgThemeTestData;

static const struct {
    const gchar *testpath;
    NkXdgThemeTestData data;
} _nk_uuid_tests_list[] = {
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/symbolic/found",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "zoom-in-symbolic",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/Adwaita/scalable/actions/zoom-in-symbolic.svg",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/symbolic/not-scalable",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "zoom-in-symbolic",
            .size = 48,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/Adwaita/48x48/actions/zoom-in-symbolic.symbolic.png",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/symbolic/found-no-symbolic",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "trophy-gold-symbolic",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/Adwaita/48x48/status/trophy-gold.png",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/theme-found/fallback",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "geany",
            .size = 16,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/hicolor/16x16/apps/geany.png",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/context/exist-match/1",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "network-wireless-signal-ok-symbolic",
            .context = "Status",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/Adwaita/scalable/status/network-wireless-signal-ok-symbolic.svg",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/context/exist-no-match",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "network-wireless-signal-ok-symbolic",
            .context = "Applications",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = NULL,
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/context/exist-match/2",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "emblem-favorite-symbolic",
            .context = "Emblems",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/Adwaita/scalable/emblems/emblem-favorite-symbolic.svg",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/symbolic/found",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "zoom-in-symbolic",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/gnome/scalable/actions/zoom-in-symbolic.svg",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/symbolic/not-scalable",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "zoom-in-symbolic",
            .size = 48,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/gnome/48x48/actions/zoom-in.png",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/symbolic/found-no-symbolic",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "trophy-gold-symbolic",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/gnome/48x48/status/trophy-gold.png",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/theme-found/fallback",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "geany",
            .size = 16,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/hicolor/16x16/apps/geany.png",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/context/exist-match/1",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "network-wireless-signal-ok-symbolic",
            .context = "Status",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/gnome/scalable/status/network-wireless-signal-ok-symbolic.svg",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/context/exist-no-match",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "network-wireless-signal-ok-symbolic",
            .context = "Applications",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = NULL,
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/context/exist-match/2",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "emblem-favorite-symbolic",
            .context = "Emblems",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/gnome/scalable/emblems/emblem-favorite-symbolic.svg",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme/threshold/found/1",
        .data = {
            .type = TYPE_ICON,
            .theme = NULL,
            .name = "geany",
            .size = 18,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/hicolor/16x16/apps/geany.png",
            .theme_test = "/usr/share/icons/hicolor/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme/threshold/found/2",
        .data = {
            .type = TYPE_ICON,
            .theme = NULL,
            .name = "pidgin",
            .size = 18,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/hicolor/16x16/apps/pidgin.png",
            .theme_test = "/usr/share/icons/hicolor/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme/size/fallback/hicolor/1",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "pidgin",
            .size = 0,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/hicolor/scalable/apps/pidgin.svg",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme/size/fallback/pixmaps/1",
        .data = {
            .type = TYPE_ICON,
            .theme = NULL,
            .name = "htop",
            .size = 19,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/pixmaps/htop.png",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme/size/fallback/pixmaps/2",
        .data = {
            .type = TYPE_ICON,
            .theme = NULL,
            .name = "debian-logo",
            .size = 0,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/pixmaps/debian-logo.png",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/size/biggest/fixed",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "edit-find-symbolic",
            .size = 0,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/Adwaita/96x96/actions/edit-find-symbolic.symbolic.png",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/size/biggest/svg",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "edit-find-symbolic",
            .size = 0,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/Adwaita/scalable/actions/edit-find-symbolic.svg",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/Adwaita/size/best-distance",
        .data = {
            .type = TYPE_ICON,
            .theme = "Adwaita",
            .name = "edit-find",
            .size = 19,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/Adwaita/22x22/actions/edit-find.png",
            .theme_test = "/usr/share/icons/Adwaita/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/size/biggest/fixed",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "edit-find-symbolic",
            .size = 0,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/gnome/96x96/actions/edit-find-symbolic.symbolic.png",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/size/biggest/svg",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "edit-find-symbolic",
            .size = 0,
            .scale = 1,
            .svg = TRUE,
            .result = "/usr/share/icons/gnome/scalable/actions/edit-find-symbolic.svg",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/gnome/size/best-distance",
        .data = {
            .type = TYPE_ICON,
            .theme = "gnome",
            .name = "edit-find",
            .size = 19,
            .scale = 1,
            .svg = FALSE,
            .result = "/usr/share/icons/gnome/22x22/actions/edit-find.png",
            .theme_test = "/usr/share/icons/gnome/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/wrong-theme/not-found",
        .data = {
            .type = TYPE_ICON,
            .theme = "do-not-exists-hopefully",
            .name = "nothing-on-earth-will-have-that-name",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = NULL,
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/icon/no-theme/not-found",
        .data = {
            .type = TYPE_ICON,
            .theme = NULL,
            .name = "nothing-on-earth-will-have-that-name",
            .size = 48,
            .scale = 1,
            .svg = TRUE,
            .result = NULL,
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/sound/found/variant",
        .data = {
            .type = TYPE_SOUND,
            .theme = "freedesktop",
            .name = "network-connectivity-established",
            .profile = "stereo",
            .result = "/usr/share/sounds/freedesktop/stereo/network-connectivity-established.oga",
            .theme_test = "/usr/share/sounds/freedesktop/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/sound/fallback/variant",
        .data = {
            .type = TYPE_SOUND,
            .theme = "freedesktop",
            .name = "bell-too-specific",
            .profile = "stereo",
            .result = "/usr/share/sounds/freedesktop/stereo/bell.oga",
            .theme_test = "/usr/share/sounds/freedesktop/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/sound/fallback/freedesktop",
        .data = {
            .type = TYPE_SOUND,
            .theme = "non-existing-i-hope",
            .name = "dialog-information",
            .profile = "stereo",
            .result = "/usr/share/sounds/freedesktop/stereo/dialog-information.oga",
            .theme_test = "/usr/share/sounds/freedesktop/index.theme",
        }
    },
    {
        .testpath = "/nkutils/xdg-theme/sound/fallback/no-index",
        .data = {
            .type = TYPE_SOUND,
            .theme = "purple",
            .name = "logout",
            .profile = "stereo",
            .result = "/usr/share/sounds/purple/logout.wav",
        }
    },
};

static void
_nk_uuid_tests_func(gconstpointer user_data)
{
    const NkXdgThemeTestData *data = user_data;

    if ( ( ( data->result != NULL ) && ( ! g_file_test(data->result, G_FILE_TEST_IS_REGULAR) ) )
         || ( ( data->theme_test != NULL ) && ( ! g_file_test(data->theme_test, G_FILE_TEST_IS_REGULAR) ) ) )
    {
        g_test_skip("Theme not installed");
        return;
    }

    gchar *file;
    switch ( data->type )
    {
    case TYPE_ICON:
        file = nk_xdg_theme_get_icon(context, data->theme, data->context, data->name, data->size, data->scale, data->svg);
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

int
main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

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
