/*
 * libnkutils/xdg-theme - Miscellaneous utilities, XDG Theme module
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
#define G_LOG_DOMAIN "libnkutils-xdg-theme"

#include <string.h>
#include <locale.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "nkutils-enum.h"
#include "nkutils-xdg-theme.h"

typedef enum {
    TYPE_ICON,
    TYPE_SOUND,
} NkXdgThemeThemeType;

struct _NkXdgThemeContext {
    gchar **dirs[2];
    gsize dirs_length[2];
    GHashTable *icon_themes;
    GHashTable *sound_themes;
};

typedef struct {
    NkXdgThemeContext *context;
    NkXdgThemeThemeType type;
    gchar *name;
    GList *subdirs;
    GList *inherits;
} NkXdgThemeTheme;

typedef enum {
    ICONDIR_TYPE_THRESHOLD = 0,
    ICONDIR_TYPE_FIXED,
    ICONDIR_TYPE_SCALABLE,
} NkXdgThemeIconDirType;

typedef enum {
    ICONDIR_CONTEXT_CUSTOM = -1,
    ICONDIR_CONTEXT_UNKNOWN = 0,
    ICONDIR_CONTEXT_ACTIONS,
    ICONDIR_CONTEXT_ANIMATIONS,
    ICONDIR_CONTEXT_APPLICATIONS,
    ICONDIR_CONTEXT_CATEGORIES,
    ICONDIR_CONTEXT_DEVICES,
    ICONDIR_CONTEXT_EMBLEMS,
    ICONDIR_CONTEXT_EMOTES,
    ICONDIR_CONTEXT_FILESYSTEMS,
    ICONDIR_CONTEXT_INTERNATIONAL,
    ICONDIR_CONTEXT_MIMETYPES,
    ICONDIR_CONTEXT_PLACES,
    ICONDIR_CONTEXT_STATUS,
    ICONDIR_CONTEXT_STOCK,
} NkXdgThemeIconDirContext;

static const gchar * const _nk_xdg_theme_icon_dir_type_names[] = {
    [ICONDIR_TYPE_THRESHOLD] = "Threshold",
    [ICONDIR_TYPE_FIXED]     = "Fixed",
    [ICONDIR_TYPE_SCALABLE]  = "Scalable",
};

static const gchar * const _nk_xdg_theme_icon_dir_context_names[] = {
    [ICONDIR_CONTEXT_UNKNOWN] = NULL,
    [ICONDIR_CONTEXT_ACTIONS]       = "Actions",
    [ICONDIR_CONTEXT_ANIMATIONS]    = "Animations",
    [ICONDIR_CONTEXT_APPLICATIONS]  = "Applications",
    [ICONDIR_CONTEXT_CATEGORIES]    = "Categories",
    [ICONDIR_CONTEXT_DEVICES]       = "Devices",
    [ICONDIR_CONTEXT_EMBLEMS]       = "Emblems",
    [ICONDIR_CONTEXT_EMOTES]        = "Emotes",
    [ICONDIR_CONTEXT_FILESYSTEMS]   = "FileSystems",
    [ICONDIR_CONTEXT_INTERNATIONAL] = "International",
    [ICONDIR_CONTEXT_MIMETYPES]     = "MimeTypes",
    [ICONDIR_CONTEXT_PLACES]        = "Places",
    [ICONDIR_CONTEXT_STATUS]        = "Status",
    [ICONDIR_CONTEXT_STOCK]         = "Stock",
};

typedef struct {
    NkXdgThemeIconDirContext context;
    const gchar *context_custom;
    gint size;
    gboolean scalable;
    gboolean svg;
} NkXdgThemeIconFindData;

typedef struct {
    const gchar *profile;
    gchar **names;
} NkXdgThemeSoundFindData;

typedef struct {
    gchar **paths;
    gint weight;
} NkXdgThemeDir;

typedef struct {
    NkXdgThemeDir base;
    NkXdgThemeIconDirType type;
    gint size;
    gint min;
    gint max;
    NkXdgThemeIconDirContext context;
    gchar *context_custom;
} NkXdgThemeIconDir;

typedef struct {
    NkXdgThemeDir base;
    gchar *profile;
} NkXdgThemeSoundDir;

static const gchar * const _nk_xdg_theme_subdirs[] = {
    [TYPE_ICON] = "icons",
    [TYPE_SOUND] = "sounds",
};

static const gchar * const _nk_xdg_theme_sections[] = {
    [TYPE_ICON] = "Icon Theme",
    [TYPE_SOUND] = "Sound Theme",
};

static const gchar *_nk_xdg_theme_icon_extensions[] = {
    ".svg",
    ".png",
    ".xpm",
    NULL
};

static const gchar *_nk_xdg_theme_sound_extensions[] = {
    ".disabled",
    ".oga",
    ".ogg",
    ".wav",
    NULL
};

static void
_nk_xdg_theme_find_dirs(NkXdgThemeContext *self, NkXdgThemeThemeType type)
{
    const gchar *subdir = _nk_xdg_theme_subdirs[type];
    gchar **dirs;
    gsize length = 0, current = 0;
    const gchar * const *system_dirs;
    system_dirs = g_get_system_data_dirs();

    while ( system_dirs[length] != NULL ) ++length; /* will give us the NULL terminator */
    ++length; /* user dir */

    dirs = g_new0(gchar *, length);

#define try_dir(dir) G_STMT_START { \
        gchar *_tmp_dir = g_build_filename(dir, subdir, NULL); \
        if ( g_file_test(_tmp_dir, G_FILE_TEST_IS_DIR) ) \
            dirs[current++] = _tmp_dir;\
        else \
            g_free(_tmp_dir);\
    } G_STMT_END

    try_dir(g_get_user_data_dir());

    const gchar * const *system_dir;
    for ( system_dir = system_dirs ; *system_dir != NULL ; ++system_dir )
        try_dir(*system_dir);

#undef try_dir

    if ( current == 0 )
    {
        g_free(dirs);
        return;
    }
    self->dirs[type] = dirs;
    self->dirs_length[type] = current;
}

static gpointer
_nk_xdg_theme_icon_subdir_new(GKeyFile *file, const gchar *subdir)
{
    GError *error = NULL;
    gint size;
    size = g_key_file_get_integer(file, subdir, "Size", &error);
    if ( error != NULL )
    {
        g_clear_error(&error);
        return NULL;
    }

    NkXdgThemeIconDir *self;
    self = g_slice_new0(NkXdgThemeIconDir);

    self->size = size;
    self->min = self->size;
    self->max = self->size;

    gchar *type;
    type = g_key_file_get_string(file, subdir, "Type", NULL);
    if ( type != NULL )
    {
        guint64 value;
        if ( nk_enum_parse(type, _nk_xdg_theme_icon_dir_type_names, G_N_ELEMENTS(_nk_xdg_theme_icon_dir_type_names), TRUE, &value) )
            self->type = value;
        g_free(type);
    }

    switch ( self->type )
    {
    case ICONDIR_TYPE_THRESHOLD:
    {
        gint threshold;
        threshold = g_key_file_get_integer(file, subdir, "Threshold", &error);
        if ( error != NULL )
        {
            threshold = 2;
            g_clear_error(&error);
        }
        self->min -= threshold;
        self->max += threshold;
    }
    break;
    case ICONDIR_TYPE_FIXED:
    break;
    case ICONDIR_TYPE_SCALABLE:
    {
        self->base.weight = 1;
        gint limit;

        limit = g_key_file_get_integer(file, subdir, "MinSize", &error);
        if ( error == NULL )
            self->min = limit;
        else
            g_clear_error(&error);

        limit = g_key_file_get_integer(file, subdir, "MaxSize", &error);
        if ( error == NULL )
            self->max = limit;
        else
            g_clear_error(&error);
    }
    break;
    default:
        g_slice_free(NkXdgThemeIconDir, self);
        g_return_val_if_reached(NULL);
    }

    gchar *context;
    context = g_key_file_get_string(file, subdir, "Context", NULL);
    if ( context != NULL )
    {
        guint64 value;
        if ( nk_enum_parse(context, _nk_xdg_theme_icon_dir_context_names, G_N_ELEMENTS(_nk_xdg_theme_icon_dir_context_names), TRUE, &value) )
        {
            self->context = value;
            g_free(context);
        }
        else
        {
            self->context = ICONDIR_CONTEXT_CUSTOM;
            self->context_custom = context;
        }
    }

    return self;
}

static void
_nk_xdg_theme_icon_subdir_free(gpointer data)
{
    NkXdgThemeIconDir *self = data;
    g_free(self->context_custom);
    g_strfreev(self->base.paths);
    g_slice_free(NkXdgThemeIconDir, self);
}

static gpointer
_nk_xdg_theme_sound_subdir_new(GKeyFile *file, const gchar *subdir)
{
    NkXdgThemeSoundDir *self;
    self = g_slice_new0(NkXdgThemeSoundDir);

    self->profile = g_key_file_get_string(file, subdir, "OutputProfile", NULL);

    return self;
}

static void
_nk_xdg_theme_sound_subdir_free(gpointer data)
{
    NkXdgThemeSoundDir *self = data;

    g_free(self->profile);

    g_strfreev(self->base.paths);
    g_slice_free(NkXdgThemeSoundDir, self);
}

static gint
_nk_xdg_theme_subdir_sort(gconstpointer a_, gconstpointer b_)
{
    const NkXdgThemeDir *a = a_;
    const NkXdgThemeDir *b = b_;
    return ( b->weight - a->weight );
}

static gpointer _nk_xdg_theme_get_theme(NkXdgThemeContext *self, NkXdgThemeThemeType type, const gchar *name);
static gboolean
_nk_xdg_theme_find(NkXdgThemeTheme *self)
{
    const gchar *section = _nk_xdg_theme_sections[self->type];
    GKeyFile *file;

    file = g_key_file_new();
    g_key_file_set_list_separator(file, ',');

    gboolean found = FALSE;
    gchar **dir;
    for ( dir = self->context->dirs[self->type] ; ( ! found ) && ( *dir != NULL ) ; ++dir )
    {
        gchar *filename;
        filename = g_build_filename(*dir, self->name, "index.theme", NULL);
        if ( g_key_file_load_from_file(file, filename, G_KEY_FILE_NONE, NULL)
             && g_key_file_has_group(file, section) )
            found = TRUE;
        g_free(filename);
    }

    if ( ! found )
        goto error;
    found = FALSE;

    gchar **subdirs, **subdir_path;
    subdirs = g_key_file_get_string_list(file, section, "Directories", NULL, NULL);
    if ( subdirs == NULL )
        goto error;
    gpointer (*subdir_new)(GKeyFile *file, const gchar *subdir);
    void (*subdir_free)(gpointer subdir);
    switch ( self->type )
    {
    case TYPE_ICON:
        subdir_new = _nk_xdg_theme_icon_subdir_new;
        subdir_free = _nk_xdg_theme_icon_subdir_free;
    break;
    case TYPE_SOUND:
        subdir_new = _nk_xdg_theme_sound_subdir_new;
        subdir_free = _nk_xdg_theme_sound_subdir_free;
    break;
    default:
        g_return_val_if_reached(FALSE);
    }

    for ( subdir_path = subdirs ; *subdir_path != NULL ; ++subdir_path )
    {
        if ( g_key_file_has_group(file, *subdir_path) )
        {
            NkXdgThemeDir *subdir;
            subdir = subdir_new(file, *subdir_path);
            if ( subdir == NULL )
                continue;

            gsize i;
            subdir->paths = g_new(gchar *, self->context->dirs_length[self->type] + 1);

            gchar **dir_;
            for ( dir_ = self->context->dirs[self->type], i = 0 ; *dir_ != NULL ; ++dir_ )
            {
                gchar *path;
                path = g_build_filename(*dir_, self->name, *subdir_path, NULL);
                if ( g_file_test(path, G_FILE_TEST_IS_DIR) )
                    subdir->paths[i++] = path;
                else
                    g_free(path);
            }
            subdir->paths[i] = NULL;

            if ( i == 0 )
                subdir_free(subdir);
            else
                self->subdirs = g_list_insert_sorted(self->subdirs, subdir, _nk_xdg_theme_subdir_sort);
        }
    }

    if ( self->subdirs == NULL )
        goto error;

    gchar **inherits;
    inherits = g_key_file_get_string_list(file, section, "Inherits", NULL, NULL);
    if ( inherits != NULL )
    {
        gchar **inherit;
        for ( inherit = inherits ; *inherit != NULL ; ++inherit )
        {
            gpointer inherited;
            inherited = _nk_xdg_theme_get_theme(self->context, self->type, *inherit);
            if ( inherited != NULL )
                self->inherits = g_list_prepend(self->inherits, inherited);
        }
        g_strfreev(inherits);
        self->inherits = g_list_reverse(self->inherits);
    }

    found = TRUE;
error:
    g_key_file_free(file);
    return found;
}

static void
_nk_xdg_theme_icon_load_theme(NkXdgThemeContext *context, GHashTable *list, NkXdgThemeThemeType type, const gchar *name)
{
    NkXdgThemeTheme *self;
    self = g_new0(NkXdgThemeTheme, 1);
    self->context = context;
    self->type = type;
    self->name = g_strdup(name);

    if ( ! _nk_xdg_theme_find(self) )
    {
        g_hash_table_insert(list, self->name, NULL);
        g_free(self);
    }
    else
        g_hash_table_insert(list, self->name, self);
}

static void
_nk_xdg_theme_theme_free(gpointer data)
{
    NkXdgThemeTheme *self = data;
    if ( self == NULL )
        return;

    void (*subdir_free)(gpointer subdir);
    switch ( self->type )
    {
    case TYPE_ICON:
        subdir_free = _nk_xdg_theme_icon_subdir_free;
    break;
    case TYPE_SOUND:
        subdir_free = _nk_xdg_theme_sound_subdir_free;
    break;
    default:
        g_return_if_reached();
    }

    g_list_free_full(self->subdirs, subdir_free);
    g_list_free(self->inherits);
    g_free(self);
}

static gpointer
_nk_xdg_theme_get_theme(NkXdgThemeContext *self, NkXdgThemeThemeType type, const gchar *name)
{
    if ( name == NULL )
        return NULL;

    GHashTable *list;
    switch ( type )
    {
    case TYPE_ICON:
        list = self->icon_themes;
    break;
    case TYPE_SOUND:
        list = self->sound_themes;
    break;
    default:
        g_return_val_if_reached(FALSE);
    }

    if ( ! g_hash_table_contains(list, name) )
        _nk_xdg_theme_icon_load_theme(self, list, type, name);
    return g_hash_table_lookup(list, name);
}

NkXdgThemeContext *
nk_xdg_theme_context_new(void)
{
    NkXdgThemeContext *self;
    self = g_new0(NkXdgThemeContext, 1);

    _nk_xdg_theme_find_dirs(self, TYPE_ICON);
    self->icon_themes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _nk_xdg_theme_theme_free);
    _nk_xdg_theme_find_dirs(self, TYPE_SOUND);
    self->sound_themes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _nk_xdg_theme_theme_free);

    return self;
}

void
nk_xdg_theme_context_free(NkXdgThemeContext *self)
{
    g_hash_table_unref(self->sound_themes);
    g_strfreev(self->dirs[TYPE_SOUND]);
    g_hash_table_unref(self->icon_themes);
    g_strfreev(self->dirs[TYPE_ICON]);
    g_free(self);
}

static gboolean
_nk_xdg_theme_get_file(NkXdgThemeTheme *self, const gchar *name, gboolean (*find_file)(NkXdgThemeTheme *self, const gchar *name, gpointer data, gchar **ret), gpointer data, gchar **ret)
{
    if ( find_file(self, name, data, ret) )
        return TRUE;

    GList *inherited;
    for ( inherited = self->inherits ; inherited != NULL ; inherited = g_list_next(inherited) )
    {
        if ( _nk_xdg_theme_get_file(inherited->data, name, find_file, data, ret) )
            return TRUE;
    }
    return FALSE;
}

static gboolean
_nk_xdg_theme_try_file(const gchar *dir, const gchar *name, const gchar *extensions[], gchar **ret)
{
    gsize i;
    for ( i = 0 ; extensions[i] != NULL ; ++i )
    {
        gchar *file;
        file = g_strconcat(dir, G_DIR_SEPARATOR_S, name, extensions[i], NULL);
        if ( g_file_test(file, G_FILE_TEST_IS_REGULAR) )
        {
            *ret = file;
            return TRUE;
        }
        g_free(file);
    }
    return FALSE;
}

static gboolean
_nk_xdg_theme_try_fallback(gchar **dirs, const gchar *extra_dir, const gchar *theme_name, const gchar *name, const gchar *extensions[], gchar **ret)
{
    gchar *themed_name = NULL;
    gsize l;
    if ( theme_name != NULL )
    {
        l = strlen(theme_name) + strlen(G_DIR_SEPARATOR_S) + strlen(name) + 1;
        themed_name = g_newa(gchar, l);
        g_snprintf(themed_name, l, "%s%c%s", theme_name, G_DIR_SEPARATOR, name);
    }

    gchar **dir;
    for ( dir = dirs ; *dir != NULL ; ++dir )
    {
        if ( ( themed_name != NULL ) && _nk_xdg_theme_try_file(*dir, themed_name, extensions, ret) )
            return TRUE;
        if ( _nk_xdg_theme_try_file(*dir, name, extensions, ret) )
            return TRUE;
    }
    if ( extra_dir == NULL )
        return FALSE;

    if ( ( themed_name != NULL ) && _nk_xdg_theme_try_file(extra_dir, themed_name, extensions, ret) )
        return TRUE;
    if ( _nk_xdg_theme_try_file(extra_dir, name, extensions, ret) )
        return TRUE;
    return FALSE;
}

static gboolean
_nk_xdg_theme_icon_find_file(NkXdgThemeTheme *self, const gchar *name, gpointer user_data, gchar **ret)
{
    NkXdgThemeIconFindData *data = user_data;

    GList *subdir_;
    for ( subdir_ = self->subdirs ; subdir_ != NULL ; subdir_ = g_list_next(subdir_) )
    {
        NkXdgThemeIconDir *subdir = subdir_->data;
        gchar **path;
        if ( ( subdir->type == ICONDIR_TYPE_SCALABLE ) && ( ! data->scalable ) )
            continue;

        if ( ( data->context != ICONDIR_CONTEXT_UNKNOWN ) && ( subdir->context != ICONDIR_CONTEXT_UNKNOWN ) )
        {
            if ( data->context != subdir->context )
                continue;
            if ( ( data->context == ICONDIR_CONTEXT_CUSTOM ) && ( g_ascii_strcasecmp(data->context_custom, subdir->context_custom) != 0 ) )
                continue;
        }

        if ( ( data->size < subdir->min ) || ( data->size > subdir->max ) )
            continue;

        for ( path = subdir->base.paths ; *path != NULL ; ++path )
        {
            if ( _nk_xdg_theme_try_file(*path, name, data->svg ? _nk_xdg_theme_icon_extensions : _nk_xdg_theme_icon_extensions + 1, ret) )
                return TRUE;
        }
    }

    return FALSE;
}

gchar *
nk_xdg_theme_get_icon(NkXdgThemeContext *self, const gchar *theme_name, const gchar *context_name, const gchar *name, gint size, gboolean scalable, gboolean svg)
{
    NkXdgThemeTheme *theme;
    gchar *file;
    theme = _nk_xdg_theme_get_theme(self, TYPE_ICON, theme_name);
    if ( theme == NULL )
        theme = _nk_xdg_theme_get_theme(self, TYPE_ICON, "hicolor");

    guint64 value;
    NkXdgThemeIconFindData data = {
        .context = ICONDIR_CONTEXT_CUSTOM,
        .context_custom = context_name,
        .size = size,
        .scalable = scalable,
        .svg = svg,
    };
    if ( nk_enum_parse(context_name, _nk_xdg_theme_icon_dir_context_names, G_N_ELEMENTS(_nk_xdg_theme_icon_dir_context_names), TRUE, &value) )
        data.context = value;

    if ( ( theme != NULL ) && _nk_xdg_theme_get_file(theme, name, _nk_xdg_theme_icon_find_file, &data, &file) )
        return file;

    if ( _nk_xdg_theme_try_fallback(self->dirs[TYPE_ICON], DATADIR G_DIR_SEPARATOR_S "pixmaps", theme_name, name, scalable ? _nk_xdg_theme_icon_extensions : _nk_xdg_theme_icon_extensions + 1, &file) )
        return file;

    if ( g_str_has_suffix(name, "-symbolic") )
    {
        gchar *no_symbolic_name;
        gsize l;
        l = strlen(name) - strlen("-symbolic") + 1;
        no_symbolic_name = g_newa(gchar, l);
        g_snprintf(no_symbolic_name, l, "%s", name);
        return nk_xdg_theme_get_icon(self, theme_name, context_name, no_symbolic_name, size, scalable, svg);
    }

    return NULL;
}

static gboolean
_nk_xdg_theme_sound_find_file(NkXdgThemeTheme *self, const gchar *name_, gpointer user_data, gchar **ret)
{
    NkXdgThemeSoundFindData *data = user_data;
    GList *subdir_;
    for ( subdir_ = self->subdirs ; subdir_ != NULL ; subdir_ = g_list_next(subdir_) )
    {
        NkXdgThemeSoundDir *subdir = subdir_->data;
        gchar **path;
        if ( g_strcmp0(data->profile, subdir->profile) != 0 )
            continue;

        for ( path = subdir->base.paths ; *path != NULL ; ++path )
        {
            gchar **name;
            for ( name = data->names ; *name != NULL ; ++name )
            {
                if ( _nk_xdg_theme_try_file(*path, *name, _nk_xdg_theme_sound_extensions, ret) )
                    return TRUE;
            }
        }
    }

    if ( data->profile == NULL )
        return FALSE;

    if ( g_strcmp0(data->profile, "stereo") == 0 )
    {
        data->profile = NULL;
        return _nk_xdg_theme_sound_find_file(self, name_, user_data, ret);
    }

    data->profile = "stereo";
    return _nk_xdg_theme_sound_find_file(self, name_, user_data, ret);
}

gchar *
nk_xdg_theme_get_sound(NkXdgThemeContext *self, const gchar *theme_name, const gchar *name, const gchar *profile, const gchar *locale)
{
    NkXdgThemeTheme *theme;
    gchar *file;
    theme = _nk_xdg_theme_get_theme(self, TYPE_SOUND, theme_name);
    if ( theme == NULL )
        theme = _nk_xdg_theme_get_theme(self, TYPE_SOUND, "freedesktop");

    NkXdgThemeSoundFindData data = {
        .profile = profile,
    };

    const gchar *c;
    gsize l;

#ifdef G_OS_WIN32
    gchar *locale_ = NULL;
#endif /* G_OS_WIN32 */
    if ( locale == NULL )
#ifdef G_OS_WIN32
        locale = locale_ = g_win32_getlocale();
#else /* ! G_OS_WIN32 */
        locale = setlocale(LC_MESSAGES, NULL);
#endif /* ! G_OS_WIN32 */
    gchar *locales[5];
    gsize locales_count = 0;

    if ( ( *locale != '\0' ) && ( g_strcmp0(locale, "C") != 0 ) )
    {
        l = strlen(locale);
        locales[locales_count] = g_newa(gchar, l + 2);
        g_snprintf(locales[locales_count++], l + 2, "%s" G_DIR_SEPARATOR_S, locale);
        if ( ( c = g_utf8_strchr(locale, -1, '@') ) != NULL )
        {
            l = (c - locale);

            locales[locales_count] = g_newa(gchar, l + 2);
            g_snprintf(locales[locales_count++], l + 2, "%.*s" G_DIR_SEPARATOR_S, (gint) l, locale);
        }
        if ( ( c = g_utf8_strchr(locale, -1, '_') ) != NULL )
        {
            l = (c - locale);
            locales[locales_count] = g_newa(gchar, l + 1);
            g_snprintf(locales[locales_count++], l + 2, "%.*s" G_DIR_SEPARATOR_S, (gint) l, locale);
        }
    }
    locales[locales_count++] = "C" G_DIR_SEPARATOR_S;
    locales[locales_count++] = "";

#ifdef G_OS_WIN32
    g_free(locale_);
#endif /* G_OS_WIN32 */

    gsize variants_count = 1;
    l = strlen(name);
    for ( c = name ; ( c = g_utf8_strchr(c, l - (c - name), '-') ) != NULL ; ++c )
        ++variants_count;

    data.names = g_newa(gchar *, ( locales_count * variants_count ) + 1);
    data.names[locales_count * variants_count] = NULL;

    gsize i, j;
    for ( i = 0 ; i < locales_count ; ++i )
    {
        gsize ll;
        ll = strlen(locales[i]);

        for ( c = name + l, j = 0 ; j < variants_count ; c = g_utf8_strrchr(name, c - name, '-'), ++j )
        {
            g_assert_nonnull(c);

            gsize sl = ll + (c - name) + 1;
            data.names[i * variants_count + j] = g_newa(gchar, sl);
            g_snprintf(data.names[i * variants_count + j], sl, "%s%s", locales[i], name);
        }
    }

    if ( ( theme != NULL ) && _nk_xdg_theme_get_file(theme, name, _nk_xdg_theme_sound_find_file, &data, &file) )
        return file;

    gchar **subname;
    for ( subname = data.names ; *subname != NULL ; ++subname )
    {
        if ( _nk_xdg_theme_try_fallback(self->dirs[TYPE_SOUND], NULL, theme_name, *subname, _nk_xdg_theme_sound_extensions, &file) )
            return file;
    }

    return NULL;
}
