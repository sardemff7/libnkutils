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

#include "nkutils-enum.h"
#include "nkutils-format-string.h"

typedef enum {
    TEMPLATE_SIMPLE,
    TEMPLATE_HEADER,
    TEMPLATE_ENTITY,
} NkGitVersionTemplates;

static const gchar * const _nk_git_version_template_names[] = {
    [TEMPLATE_SIMPLE] = "simple",
    [TEMPLATE_HEADER] = "header",
    [TEMPLATE_ENTITY] = "entity",
};

static const gchar * const _nk_git_version_templates[] = {
    [TEMPLATE_SIMPLE] = "${describe}\n${branch}\n",
    [TEMPLATE_HEADER] =
        "/* File generated by nk-git-version */"

        "${describe:+"
        "\n#define NK_GIT_COMMIT \"${describe}\""
        "\n"
            "${branch:!#undef NK_GIT_BRANCH}"
            "${branch:+#define NK_GIT_BRANCH \"${branch}\"}"
        "\n#define NK_GIT_VERSION NK_GIT_COMMIT${branch:+ \" (\" NK_GIT_BRANCH \")\"}"
        "\n#define NK_PACKAGE_VERSION PACKAGE_VERSION \" - \" NK_GIT_VERSION"
        "}"

        "${describe:!"
        "\n#undef NK_GIT_COMMIT"
        "\n#undef NK_GIT_BRANCH"
        "\n#undef NK_GIT_VERSION"
        "\n#define NK_PACKAGE_VERSION PACKAGE_VERSION"
        "}"

        "\n",
    [TEMPLATE_ENTITY] =
        "<!-- File generated by nk-git-version -->"

        "\n<!ENTITY NK_GIT_COMMIT \"${describe}\">"
        "\n<!ENTITY NK_GIT_BRANCH \"${branch}\">"
        "\n<!ENTITY NK_GIT_VERSION \"${describe:+&NK_GIT_COMMIT;}${branch:+ (&NK_GIT_BRANCH;)}\">"
        "\n<!ENTITY NK_PACKAGE_VERSION \"&PACKAGE_VERSION;${describe:+ - &NK_GIT_VERSION;}\">"

        "\n",
};

typedef enum {
    DESCRIBE,
    BRANCH,
} NkGitVersionTokens;

static const gchar * const _nk_git_version_tokens[] = {
    [DESCRIBE] = "describe",
    [BRANCH]   = "branch",
};

static gchar ** _nk_git_version_token_commands[] = {
    [DESCRIBE] = (gchar *[]) { "describe", "--always", "--tags", "--dirty", NULL },
    [BRANCH]   = (gchar *[]) { "describe", "--always", "--tags", "--all", NULL },
};

#define NK_GIT_VERSION_NUM_TOKENS (G_N_ELEMENTS(_nk_git_version_tokens))

typedef struct {
    gchar *data[NK_GIT_VERSION_NUM_TOKENS];
} NkGitVersionData;


static gchar *
_nk_git_version_run_git(gchar *git, gchar *work_tree, gchar **token_args)
{
    gsize size = g_strv_length(token_args);
    gsize i = 0, j;
    gchar **args = g_new0(gchar *, size + 3 + 1);
    args[i++] = g_strdup(git);
    args[i++] = g_strdup("-C");
    args[i++] = g_strdup(work_tree);
    for ( j = 0 ; j < size ; ++j )
        args[i++] = g_strdup(token_args[j]);
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

static GVariant *
_nk_git_version_replace_reference(G_GNUC_UNUSED const gchar *name, guint64 value, gpointer user_data)
{
    NkGitVersionData *data = user_data;
    g_return_val_if_fail(value < NK_GIT_VERSION_NUM_TOKENS, NULL);
    return ( data->data[value] == NULL ) ? NULL : g_variant_new_string(data->data[value]);
}

int
main(int argc, char *argv[])
{
    if ( argc < 5 )
    {
        g_print("Usage: %s {header|entity|simple} {-|<output file>} <git work tree> <git>\n", argv[0]);
        return 1;
    }

    gchar *template = argv[1];
    NkFormatString *format_string = NULL;
    gchar *output_file = argv[2];
    gchar *work_tree = argv[3];
    gchar *git_dir = NULL;
    gchar *git = argv[4];

    gint ret = 10;
    gchar *old_contents = NULL;
    gchar *new_contents = NULL;
    GError *error = NULL;

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

    guint64 template_value;
    if ( ! nk_enum_parse(template, _nk_git_version_template_names, G_N_ELEMENTS(_nk_git_version_template_names), NK_ENUM_MATCH_FLAGS_IGNORE_CASE, &template_value) )
    {
        ret = 5;
        g_warning("Unknown template: %s", template);
        goto fail;
    }

    guint64 used_tokens;
    format_string = nk_format_string_parse_enum(g_strdup(_nk_git_version_templates[template_value]), '$', _nk_git_version_tokens, NK_GIT_VERSION_NUM_TOKENS, &used_tokens, &error);
    if ( format_string == NULL )
    {
        g_warning("Could not parse format string: %s", error->message);
        ret = 5;
        goto fail;
    }


    NkGitVersionData data = { .data = { NULL } };

    git_dir = g_build_filename(work_tree, ".git", NULL);
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
    else if ( ! _nk_git_version_get_data(&data, used_tokens, git, work_tree) )
        goto fail;

    new_contents = nk_format_string_replace(format_string, _nk_git_version_replace_reference, &data);

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
    g_free(old_contents);
    g_free(new_contents);
    if ( format_string != NULL )
        nk_format_string_unref(format_string);
    g_free(git_dir);
    return ret;
}
