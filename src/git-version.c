/*
 * libnkutils/git-version - Miscellaneous utilities, Git version module
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
#define G_LOG_DOMAIN "nk-git-version"

#include <string.h>

#include <glib.h>

typedef struct {
    gchar *commit;
    gchar *branch;
} NkGitVersionInfo;

typedef enum {
    DESCRIBE,
    BRANCH,
} NkGitVersionTokens;

static gchar * _nk_git_version_token_commands[] = {
    [DESCRIBE] = "--dirty",
    [BRANCH]   = "--all",
};

#define NK_GIT_VERSION_NUM_TOKENS (G_N_ELEMENTS(_nk_git_version_token_commands))

typedef struct {
    gchar *data[NK_GIT_VERSION_NUM_TOKENS];
} NkGitVersionData;


static gchar *
_nk_git_version_run_git(gchar *git, gchar *work_tree, gchar *arg)
{
    gsize size = 4;
    gsize i = 0;
    gchar **args = g_new0(gchar *, size + 3 + 1);
    args[i++] = g_strdup(git);
    args[i++] = g_strdup("-C");
    args[i++] = g_strdup(work_tree);
    args[i++] = g_strdup("describe");
    args[i++] = g_strdup("--always");
    args[i++] = g_strdup("--tags");
    args[i++] = g_strdup(arg);
    args[i] = NULL;

    gchar *out = NULL;
    gchar *err = NULL;
    gint status;
    GError *error = NULL;

    if ( ! g_spawn_sync(NULL, args, NULL, G_SPAWN_DEFAULT, NULL, NULL, &out, &err, &status, &error) )
    {
        g_warning("Could not run Git: %s", error->message);
        return NULL;
    }
    g_strfreev(args);

    if ( ! g_spawn_check_exit_status(status, &error) )
    {
        g_warning("Git exited with an error: %s", error->message);
        g_warning("    %s", err);
        g_free(out);
        g_free(err);
        return NULL;
    }

    g_free(err);
    return ( out == NULL ) ? NULL : g_strstrip(out);
}

static gboolean
_nk_git_version_get_data(NkGitVersionData *data, guint64 used_tokens, gchar *git, gchar *work_tree)
{
    guint64 token;
    for ( token = 0 ; token < NK_GIT_VERSION_NUM_TOKENS ; ++token )
    {
        if ( ( used_tokens & (1 << token) ) == 0 )
            continue;

        gchar *value;
        value = _nk_git_version_run_git(git, work_tree, _nk_git_version_token_commands[token]);
        if ( value == NULL )
            return FALSE;

        switch ( (NkGitVersionTokens) token )
        {
        case BRANCH:
            if ( g_str_has_prefix(value, "heads/") )
            {
                gsize i, o;
                o = strlen("heads/");
                for ( i = 0 ; value[i] != '\0' ; ++i )
                    value[i] = value[o + i];
                value[i] = '\0';
            }
            else
                value = (g_free(value), NULL);
        break;
        case DESCRIBE:
        break;
        }
        data->data[token] = value;
    }

    return TRUE;
}

static gchar *
_nk_git_version_make_header(NkGitVersionInfo *info)
{
    GString *string;

    string = g_string_sized_new(300);

    if ( info->commit == NULL )
        g_string_append(string, "#undef NK_GIT_COMMIT\n");
    else
        g_string_append_printf(string, "#define NK_GIT_COMMIT \"%s\"\n", info->commit);

    if ( info->branch == NULL )
        g_string_append(string, "#undef NK_GIT_BRANCH\n");
    else
        g_string_append_printf(string, "#define NK_GIT_BRANCH \"%s\"\n", info->branch);

    if ( info->commit != NULL )
    {
        if ( info->branch != NULL )
            g_string_append(string, "#define NK_GIT_VERSION NK_GIT_COMMIT \" (\" NK_GIT_BRANCH \")\"\n");
        else
            g_string_append(string, "#define NK_GIT_VERSION NK_GIT_COMMIT\n");
        g_string_append(string, "#define NK_PACKAGE_VERSION PACKAGE_VERSION \" - \" NK_GIT_VERSION\n");
    }
    else
    {
        g_string_append(string, "#undef NK_GIT_VERSION\n");
        g_string_append(string, "#define NK_PACKAGE_VERSION PACKAGE_VERSION\n");
    }

    return g_string_free(string, FALSE);
}

static gchar *
_nk_git_version_make_entity(NkGitVersionInfo *info)
{
    GString *string;

    string = g_string_sized_new(300);

    if ( info->commit == NULL )
        g_string_append(string, "<!ENTITY NK_GIT_COMMIT \"\">\n");
    else
        g_string_append_printf(string, "<!ENTITY NK_GIT_COMMIT \"%s\">\n", info->commit);

    if ( info->branch == NULL )
        g_string_append(string, "<!ENTITY NK_GIT_BRANCH \"\">\n");
    else
        g_string_append_printf(string, "<!ENTITY NK_GIT_BRANCH \"%s\">\n", info->branch);

    if ( info->commit != NULL )
    {
        if ( info->branch != NULL )
            g_string_append(string, "<!ENTITY NK_GIT_VERSION \"&NK_GIT_COMMIT; (&NK_GIT_BRANCH;)\">\n");
        else
            g_string_append(string, "<!ENTITY NK_GIT_VERSION \"&NK_GIT_COMMIT;\">\n");
        g_string_append(string, "<!ENTITY NK_PACKAGE_VERSION \"&PACKAGE_VERSION; - &NK_GIT_VERSION;\">\n");
    }
    else
    {
        g_string_append(string, "<!ENTITY NK_GIT_VERSION \"\">\n");
        g_string_append(string, "<!ENTITY NK_PACKAGE_VERSION \"&PACKAGE_VERSION;\">\n");
    }

    return g_string_free(string, FALSE);
}

static gchar *
_nk_git_version_make_simple(NkGitVersionInfo *info)
{
    GString *string;

    string = g_string_sized_new(100);

    if ( info->commit == NULL )
        g_string_append(string, "\n");
    else
        g_string_append_printf(string, "%s\n", info->commit);

    if ( info->branch == NULL )
        g_string_append(string, "\n");
    else
        g_string_append_printf(string, "%s\n", info->branch);

    return g_string_free(string, FALSE);
}

int
main(int argc, char *argv[])
{
    if ( argc < 6 )
        return 1;

    gchar *out_type = argv[1];
    gchar *output_file = argv[2];
    gchar *work_tree = argv[3];
    gchar *git_dir = argv[4];
    gchar *git = argv[5];

    gint ret = 10;
    gchar *old_contents = NULL;
    gchar *new_contents = NULL;
    GError *error = NULL;

    NkGitVersionInfo info = {
        .commit = NULL,
    };

    if ( g_strcmp0(output_file, "-") == 0 )
        output_file = NULL;
    else if ( g_file_test(output_file, G_FILE_TEST_IS_REGULAR) )
    {
        if ( ! g_file_get_contents(output_file, &old_contents, NULL, &error) )
        {
            g_warning("Could not read old file: %s", error->message);
            ret = 2;
            goto fail;
        }
    }
    else if ( g_file_test(output_file, G_FILE_TEST_EXISTS) )
    {
        g_warning("Target file '%s' exists already but is not a regular file", output_file);
        ret = 2;
        goto fail;
    }

    NkGitVersionData data = { .data = { NULL } };

    if ( ! g_file_test(git_dir, G_FILE_TEST_IS_DIR) )
    {
        if ( g_file_test(git_dir, G_FILE_TEST_EXISTS) )
        {
            ret = 3;
            g_warning("Git directory '%s' exists but is not a directory", git_dir);
            goto fail;
        }
    }
    else if ( ! g_file_test(git, G_FILE_TEST_IS_EXECUTABLE) )
    {
        if ( g_file_test(git, G_FILE_TEST_EXISTS) )
        {
            ret = 4;
            g_warning("Git executable '%s' exists but is not an executable", git);
            goto fail;
        }
    }
    else if ( ! _nk_git_version_get_data(&data, ~0LL, git, work_tree) )
        goto fail;

    info.commit = data.data[DESCRIBE];
    info.branch = data.data[BRANCH];

    switch ( g_utf8_get_char(out_type) )
    {
    case 'h':
        new_contents = _nk_git_version_make_header(&info);
    break;
    case 'e':
        new_contents = _nk_git_version_make_entity(&info);
    break;
    case 's':
        new_contents = _nk_git_version_make_simple(&info);
    break;
    default:
        ret = 5;
        g_warning("Wrong output type: %s", out_type);
        goto fail;
    }

    if ( g_strcmp0(old_contents, new_contents) != 0 )
    {
        if ( output_file == NULL )
            g_print("%s", new_contents);
        else if ( ! g_file_set_contents(output_file, new_contents, -1, &error) )
        {
            g_warning("Could not write new git version file: %s", error->message);
            ret = 11;
            goto fail;
        }
    }

    ret = 0;

fail:
    g_clear_error(&error);
    g_free(info.commit);
    g_free(info.branch);
    g_free(old_contents);
    g_free(new_contents);
    return ret;
}
