/*
 * libnkutils/bindings - Miscellaneous utilities, bindings module
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
#define G_LOG_DOMAIN "libnkutils-bindings"

#include <errno.h>
#include <locale.h>
#include <string.h>

#include <glib.h>

#include <xkbcommon/xkbcommon.h>
#ifdef NK_XKBCOMMON_HAS_COMPOSE
#include <xkbcommon/xkbcommon-compose.h>
#endif /* NK_XKBCOMMON_HAS_COMPOSE */

#include "nkutils-enum.h"
#include "nkutils-bindings.h"

#define NK_BINDINGS_MAX_ALIASES 8 /* For Alt */

struct _NkBindings {
    struct xkb_context *context;
    struct xkb_keymap  *keymap;
    struct xkb_state   *state;
    xkb_mod_index_t     modifiers[NK_BINDINGS_NUM_MODIFIERS][NK_BINDINGS_MAX_ALIASES];
#ifdef NK_XKBCOMMON_HAS_COMPOSE
    struct {
        struct xkb_compose_table *table;
        struct xkb_compose_state *state;
    } compose;
#endif /* NK_XKBCOMMON_HAS_COMPOSE */
    guint64 double_click_delay;
    GList *scopes;
    GList *on_release;
};

typedef struct {
    guint id;
    GHashTable *bindings;
} NkBindingsScope;

typedef struct {
    GHashTable *keycodes;
    GHashTable *keysyms;
    GHashTable *buttons;
} NkBindingsBindingGroup;

typedef struct {
    NkBindingsCallback callback;
    gpointer user_data;
    GDestroyNotify notify;
} NkBindingsBindingBase;

typedef struct {
    NkBindingsBindingBase base;
} NkBindingsBindingPress;

typedef struct {
    NkBindingsBindingBase base;
    GList *link;
} NkBindingsBindingRelease;

typedef struct {
    guint scope;
    NkBindingsBindingPress press;
    NkBindingsBindingRelease release;
} NkBindingsBinding;

typedef struct {
    NkBindingsBinding click;
    NkBindingsBinding dclick;
    guint64 last_timestamp;
} NkBindingsBindingMouse;

static const gchar const *_nk_bindings_modifiers_names[] = {
    [NK_BINDINGS_MODIFIER_SHIFT   + NK_BINDINGS_NUM_MODIFIERS * 0] = "shift",
    [NK_BINDINGS_MODIFIER_CONTROL + NK_BINDINGS_NUM_MODIFIERS * 0] = "control",
    [NK_BINDINGS_MODIFIER_ALT     + NK_BINDINGS_NUM_MODIFIERS * 0] = "alt",
    [NK_BINDINGS_MODIFIER_SUPER   + NK_BINDINGS_NUM_MODIFIERS * 0] = "super",
    [NK_BINDINGS_MODIFIER_META    + NK_BINDINGS_NUM_MODIFIERS * 0] = "meta",
    [NK_BINDINGS_MODIFIER_HYPER   + NK_BINDINGS_NUM_MODIFIERS * 0] = "hyper",
    /* Allow a few aliases */
    [NK_BINDINGS_MODIFIER_SHIFT   + NK_BINDINGS_NUM_MODIFIERS * 1] = "shift_l",
    [NK_BINDINGS_MODIFIER_CONTROL + NK_BINDINGS_NUM_MODIFIERS * 1] = "control_l",
    [NK_BINDINGS_MODIFIER_ALT     + NK_BINDINGS_NUM_MODIFIERS * 1] = "alt_l",
    [NK_BINDINGS_MODIFIER_SUPER   + NK_BINDINGS_NUM_MODIFIERS * 1] = "super_l",
    [NK_BINDINGS_MODIFIER_META    + NK_BINDINGS_NUM_MODIFIERS * 1] = "meta_l",
    [NK_BINDINGS_MODIFIER_HYPER   + NK_BINDINGS_NUM_MODIFIERS * 1] = "hyper_l",
    [NK_BINDINGS_MODIFIER_SHIFT   + NK_BINDINGS_NUM_MODIFIERS * 2] = "shift_r",
    [NK_BINDINGS_MODIFIER_CONTROL + NK_BINDINGS_NUM_MODIFIERS * 2] = "control_r",
    [NK_BINDINGS_MODIFIER_ALT     + NK_BINDINGS_NUM_MODIFIERS * 2] = "alt_r",
    [NK_BINDINGS_MODIFIER_SUPER   + NK_BINDINGS_NUM_MODIFIERS * 2] = "super_r",
    [NK_BINDINGS_MODIFIER_META    + NK_BINDINGS_NUM_MODIFIERS * 2] = "meta_r",
    [NK_BINDINGS_MODIFIER_HYPER   + NK_BINDINGS_NUM_MODIFIERS * 2] = "hyper_r",
    [NK_BINDINGS_MODIFIER_SHIFT   + NK_BINDINGS_NUM_MODIFIERS * 3] = "",
    [NK_BINDINGS_MODIFIER_CONTROL + NK_BINDINGS_NUM_MODIFIERS * 3] = "ctrl",
    [NK_BINDINGS_MODIFIER_ALT     + NK_BINDINGS_NUM_MODIFIERS * 3] = "altgr",
    [NK_BINDINGS_MODIFIER_SUPER   + NK_BINDINGS_NUM_MODIFIERS * 3] = "logo",
    [NK_BINDINGS_MODIFIER_META    + NK_BINDINGS_NUM_MODIFIERS * 3] = "",
    [NK_BINDINGS_MODIFIER_HYPER   + NK_BINDINGS_NUM_MODIFIERS * 3] = "",
};

GQuark
nk_bindings_error(void)
{
    return g_quark_from_static_string("nk_bindings_error-quark");
}

static void
_nk_bindings_scope_free(gpointer data)
{
    NkBindingsScope *scope = data;

    g_hash_table_unref(scope->bindings);
    g_slice_free(NkBindingsScope, scope);
}

static void
_nk_bindings_binding_group_free(gpointer data)
{
    NkBindingsBindingGroup *group = data;

    g_hash_table_unref(group->keycodes);
    g_hash_table_unref(group->keysyms);
    g_hash_table_unref(group->buttons);

    g_slice_free(NkBindingsBindingGroup, group);
}

static void
_nk_bindings_binding_notify(NkBindingsBinding *binding)
{
    if ( binding->press.base.notify != NULL )
        binding->press.base.notify(binding->press.base.user_data);
    if ( binding->release.base.notify != NULL )
        binding->release.base.notify(binding->release.base.user_data);
}

static void
_nk_bindings_binding_free(gpointer data)
{
    NkBindingsBinding *binding = data;

    _nk_bindings_binding_notify(binding);

    g_slice_free(NkBindingsBinding, binding);
}

static void
_nk_bindings_binding_mouse_free(gpointer data)
{
    NkBindingsBindingMouse *binding = data;

    _nk_bindings_binding_notify(&binding->click);
    _nk_bindings_binding_notify(&binding->dclick);

    g_slice_free(NkBindingsBindingMouse, binding);
}

static gint
_nk_bindings_scope_compare(gconstpointer a, gconstpointer b)
{
    const NkBindingsScope *sa = a, *sb = b;
    return ( sb->id - sa->id );
}

static NkBindingsBindingGroup *
_nk_bindings_get_group(NkBindings *self, guint scope_id, xkb_mod_mask_t mask)
{
    GList *link;
    NkBindingsScope *scope, cscope = { .id = scope_id };
    link = g_list_find_custom(self->scopes, &cscope, _nk_bindings_scope_compare);
    if ( link != NULL )
        scope = link->data;
    else
    {
        scope = g_slice_new(NkBindingsScope);
        scope->id = scope_id;
        scope->bindings = g_hash_table_new_full(NULL, NULL, NULL, _nk_bindings_binding_group_free);
        self->scopes = g_list_insert_sorted(self->scopes, scope, _nk_bindings_scope_compare);
    }

    NkBindingsBindingGroup *group;
    group = g_hash_table_lookup(scope->bindings, GUINT_TO_POINTER(mask));
    if ( group == NULL )
    {
        group = g_slice_new(NkBindingsBindingGroup);
        group->keycodes = g_hash_table_new_full(NULL, NULL, NULL, _nk_bindings_binding_free);
        group->keysyms = g_hash_table_new_full(NULL, NULL, NULL, _nk_bindings_binding_free);
        group->buttons = g_hash_table_new_full(NULL, NULL, NULL, _nk_bindings_binding_mouse_free);
        g_hash_table_insert(scope->bindings, GUINT_TO_POINTER(mask), group);
    }

    return group;
}

static gboolean
_nk_bindings_parse_modifier(const gchar *string, xkb_mod_mask_t *mask)
{
    guint64 value;

    if ( ! nk_enum_parse(string, _nk_bindings_modifiers_names, G_N_ELEMENTS(_nk_bindings_modifiers_names), TRUE, &value) )
        return FALSE;

    value %= NK_BINDINGS_NUM_MODIFIERS;

    *mask |= (1 << value);
    return TRUE;
}

gboolean
nk_bindings_add_binding(NkBindings *self, guint scope, const gchar *string, NkBindingsCallback callback, gpointer user_data, GDestroyNotify notify, GError **error)
{
    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    gboolean on_release = FALSE;
    xkb_mod_mask_t last_mask = 0;
    xkb_mod_mask_t mask = 0;
    xkb_keysym_t last_keysym = XKB_KEY_NoSymbol;
    xkb_keysym_t keysym = XKB_KEY_NoSymbol;
    xkb_keycode_t keycode = XKB_KEYCODE_INVALID;
    guint button = 0;
    gboolean double_click = FALSE;

    const gchar *w = string;

    enum {
        NK_BINDINGS_MODE_NONE,
        NK_BINDINGS_MODE_EXCLAMATION  = '!',
        NK_BINDINGS_MODE_GTK   = '>',
        NK_BINDINGS_MODE_PLUS  = '+',
        NK_BINDINGS_MODE_MINUS = '-',
    } mode = NK_BINDINGS_MODE_NONE;

    if ( g_utf8_get_char(w) == '!' )
    {
        w = g_utf8_next_char(w);
        on_release = TRUE;
        mode = NK_BINDINGS_MODE_EXCLAMATION;
    }

    gsize l;
    gchar *tmp;
    const gchar *e, *s;
    l = strlen(w);
    tmp = g_newa(gchar, l + 1);
    e = w + l;
    s = w;
    for ( e = w + l ; w < e ; w = g_utf8_next_char(w) )
    {
        gunichar wc = g_utf8_get_char(w);
        switch ( wc )
        {
        case '!':
            g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': '!' must be the first character", string);
            return FALSE;
        case '<':
            if ( ( mode != NK_BINDINGS_MODE_NONE ) && ( mode != NK_BINDINGS_MODE_GTK ) )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': you cannot mix syntaxes", string);
                return FALSE;
            }
            mode = NK_BINDINGS_MODE_GTK;
            s = g_utf8_next_char(w);
            continue;
        case '>':
        case '+':
        case '-':
            if ( mode == NK_BINDINGS_MODE_NONE )
            {
                if ( wc == '>' )
                {
                    g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': expected identifier or '<', got '>'", string);
                    return FALSE;
                }
                mode = wc;
            }
            else if ( mode == NK_BINDINGS_MODE_EXCLAMATION )
            {
                if ( wc != '>' )
                    mode = wc;
            }
            if ( mode != wc )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': you cannot mix syntaxes", string);
                return FALSE;
            }
            if ( s == w )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': cannot have an empty modifier", string);
                return FALSE;
            }

            g_snprintf(tmp, w - s + 1, "%s", s);
            if ( mode != NK_BINDINGS_MODE_GTK )
            {
                last_mask = mask;
                last_keysym = xkb_keysym_from_name(tmp, XKB_KEYSYM_NO_FLAGS);
            }
            if ( ! _nk_bindings_parse_modifier(tmp, &mask) )
            {
                if ( g_ascii_strcasecmp(tmp, "release") == 0 )
                {
                    if ( on_release )
                    {
                        g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': cannot have on-release syntax twice", string);
                        return FALSE;
                    }
                    on_release = TRUE;
                }
                else
                {
                    g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': cannot have two non-modifiers", string);
                    return FALSE;
                }
            }
            s = g_utf8_next_char(w);
        break;
        case '[':
            w = g_utf8_prev_char(e);
        break;
        default:
            if ( g_unichar_isalnum(wc) || ( wc == '_' ) )
                break;
            g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': unexpected character '%.*s'", string, (gint) ( g_utf8_next_char(w) - w ),  w);
            return FALSE;
        }
    }
    if ( s < e )
    {
        if ( g_utf8_get_char(s) == '[' )
        {
            guint64 code;
            gchar *ce;
            errno = 0;
            s = g_utf8_next_char(s);
            code = g_ascii_strtoull(s, &ce, 10);
            if ( s == ce )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': could not parse keycode", string);
                return FALSE;
            }
            else if ( ( g_utf8_get_char(ce) != ']' ) || ( e != g_utf8_next_char(ce) ) )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': keycode must be the end of the binding string, enclosed in squared brackets '[]'", string);
                return FALSE;
            }
            else if ( ! xkb_keycode_is_legal_ext(code) )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': wrong keycode value %"G_GINT64_MODIFIER"u", string, code);
                return FALSE;
            }
            keycode = code;
        }
        else if ( g_ascii_strncasecmp(s, "Mouse", strlen("Mouse")) == 0 )
        {
            s += strlen("Mouse");
            if ( g_unichar_toupper(g_utf8_get_char(s)) == 'D' )
            {
                s = g_utf8_next_char(s);
                double_click = TRUE;
            }
            guint64 code;
            gchar *ce;
            errno = 0;
            code = g_ascii_strtoull(s, &ce, 10);
            if ( s == ce )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': could not parse mouse button number", string);
                return FALSE;
            }
            else if ( e != ce )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_SYNTAX, "Syntax error in binding '%s': keycode must be the end of the binding string", string);
                return FALSE;
            }
            button = code;
        }
        else
            keysym = xkb_keysym_from_name(s, XKB_KEYSYM_NO_FLAGS);
    }

    NkBindingsBindingGroup *group;
    group = _nk_bindings_get_group(self, scope, mask);

    NkBindingsBinding *binding = NULL;
    if ( button != 0 )
    {
        NkBindingsBindingMouse *mouse_binding;
        mouse_binding = g_hash_table_lookup(group->buttons, GUINT_TO_POINTER(button));
        if ( mouse_binding == NULL )
        {
            mouse_binding = g_slice_new0(NkBindingsBindingMouse);
            g_hash_table_insert(group->buttons, GUINT_TO_POINTER(button), mouse_binding);
        }

        if ( double_click )
            binding = &mouse_binding->dclick;
        else
            binding = &mouse_binding->click;
    }
    else if ( keycode != XKB_KEYCODE_INVALID )
    {
        binding = g_hash_table_lookup(group->keycodes, GUINT_TO_POINTER(keycode));
        if ( binding == NULL )
        {
            binding = g_slice_new0(NkBindingsBinding);
            g_hash_table_insert(group->keycodes, GUINT_TO_POINTER(keycode), binding);
        }
    }
    else
    {
        if ( keysym == XKB_KEY_NoSymbol )
        {
            if  ( last_keysym == XKB_KEY_NoSymbol )
            {
                g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_ALREADY_REGISTERED, "No keysym found for binding '%s'", string);
                return FALSE;
            }
            keysym = last_keysym;
            mask = last_mask;
        }
        binding = g_hash_table_lookup(group->keysyms, GUINT_TO_POINTER(keysym));
        if ( binding == NULL )
        {
            binding = g_slice_new0(NkBindingsBinding);
            g_hash_table_insert(group->keysyms, GUINT_TO_POINTER(keysym), binding);
        }
    }

    NkBindingsBindingBase *base = on_release ? &binding->release.base : &binding->press.base;

    if ( base->callback != NULL )
    {
        g_set_error(error, NK_BINDINGS_ERROR, NK_BINDINGS_ERROR_ALREADY_REGISTERED, "There is already a binding matching '%s'", string);
        return FALSE;
    }

    binding->scope = scope;
    base->callback = callback;
    base->user_data = user_data;
    base->notify = notify;

    return TRUE;
}

static gboolean
_nk_bindings_binding_trigger(NkBindings *self, NkBindingsBinding *binding, gboolean trigger)
{
    if ( binding == NULL )
        return FALSE;

    gboolean handled = FALSE;
    gboolean has_press = ( binding->press.base.callback != NULL );
    if ( trigger && has_press )
        handled = binding->press.base.callback(binding->scope, binding->press.base.user_data);
    if ( ( binding->release.link == NULL ) && ( binding->release.base.callback != NULL ) && ( handled || ( ! has_press ) ) )
        binding->release.link = self->on_release = g_list_prepend(self->on_release, binding);

    return ( handled || ( binding->release.link != NULL ) );
}

static gboolean
_nk_bindings_try_key_bindings(NkBindings *self, xkb_mod_mask_t effective, xkb_mod_mask_t not_consumed, xkb_keycode_t keycode, xkb_keysym_t keysym, gboolean trigger)
{
    GList *scope_;
    for ( scope_ = self->scopes ; scope_ != NULL ; scope_ = g_list_next(scope_) )
    {
        NkBindingsScope *scope = scope_->data;
        NkBindingsBindingGroup *group;
        if ( ( group = g_hash_table_lookup(scope->bindings, GUINT_TO_POINTER(effective)) ) != NULL )
        {
            if ( _nk_bindings_binding_trigger(self, g_hash_table_lookup(group->keycodes, GUINT_TO_POINTER(keycode)), trigger) )
                return TRUE;
        }
        if ( ( keysym != XKB_KEY_NoSymbol ) && ( ( group = g_hash_table_lookup(scope->bindings, GUINT_TO_POINTER(not_consumed)) ) != NULL ) )
        {
            if ( _nk_bindings_binding_trigger(self, g_hash_table_lookup(group->keysyms, GUINT_TO_POINTER(keysym)), trigger) )
                return TRUE;
        }
    }

    return FALSE;
}

static gboolean
_nk_bindings_try_button_bindings(NkBindings *self, xkb_mod_mask_t mask, guint button, guint64 timestamp)
{
    NkBindingsBindingMouse *mouse_binding = NULL;
    GList *scope_;
    for ( scope_ = self->scopes ; scope_ != NULL ; scope_ = g_list_next(scope_) )
    {
        NkBindingsScope *scope = scope_->data;

        NkBindingsBindingGroup *group;
        group = g_hash_table_lookup(scope->bindings, GUINT_TO_POINTER(mask));
        if ( group == NULL )
            continue;

        mouse_binding = g_hash_table_lookup(group->buttons, GUINT_TO_POINTER(button));
        if ( mouse_binding == NULL )
            continue;

        if ( ( timestamp - mouse_binding->last_timestamp ) < self->double_click_delay )
        {
            if ( _nk_bindings_binding_trigger(self, &mouse_binding->dclick, TRUE) )
                break;
        }
        if ( _nk_bindings_binding_trigger(self, &mouse_binding->click, TRUE) )
            break;
    }
    if ( mouse_binding == NULL )
        return FALSE;

    mouse_binding->last_timestamp = timestamp;
    return TRUE;
}

static void
_nk_bindings_find_modifier(NkBindings *self, NkBindingsModifiers modifier, ...)
{
    va_list names;
    const gchar *name;
    xkb_mod_index_t i, *m = self->modifiers[modifier];
    va_start(names, modifier);
    while ( ( name = va_arg(names, const gchar *) ) != NULL )
    {
        i = xkb_keymap_mod_get_index(self->keymap, name);
        if ( i != XKB_MOD_INVALID )
            *m++ = i;
    }
    *m = XKB_MOD_INVALID;
    va_end(names);
}

NkBindings *
nk_bindings_new(struct xkb_context *context, struct xkb_keymap *keymap, struct xkb_state *state)
{
    NkBindings *self;

    self = g_new0(NkBindings, 1);
    self->context = xkb_context_ref(context);
    nk_bindings_update_keymap(self, keymap, state);

#ifdef NK_XKBCOMMON_HAS_COMPOSE
    self->compose.table = xkb_compose_table_new_from_locale(self->context, setlocale(LC_CTYPE, NULL), 0);
    if ( self->compose.table != NULL )
        self->compose.state = xkb_compose_state_new(self->compose.table, 0);
#endif /* NK_XKBCOMMON_HAS_COMPOSE */

    _nk_bindings_find_modifier(self, NK_BINDINGS_MODIFIER_SHIFT, XKB_MOD_NAME_SHIFT, NULL);
    _nk_bindings_find_modifier(self, NK_BINDINGS_MODIFIER_CONTROL, XKB_MOD_NAME_CTRL, NULL);
    _nk_bindings_find_modifier(self, NK_BINDINGS_MODIFIER_ALT, XKB_MOD_NAME_ALT, "Alt", "LAlt", "RAlt", "AltGr", "Mod5", "LevelThree", NULL);
    _nk_bindings_find_modifier(self, NK_BINDINGS_MODIFIER_META, "Meta", NULL);
    _nk_bindings_find_modifier(self, NK_BINDINGS_MODIFIER_SUPER, XKB_MOD_NAME_LOGO, "Super", NULL);
    _nk_bindings_find_modifier(self, NK_BINDINGS_MODIFIER_HYPER, "Hyper", NULL);

    self->double_click_delay = 200;

    return self;
}

static void _nk_bindings_free_on_release(NkBindings *self, gboolean trigger);
void
nk_bindings_free(NkBindings *self)
{
    if ( self == NULL )
        return;

    _nk_bindings_free_on_release(self, FALSE);

    g_list_free_full(self->scopes, _nk_bindings_scope_free);

    xkb_keymap_unref(self->keymap);
    xkb_state_unref(self->state);

    xkb_context_unref(self->context);

    g_free(self);
}

void
nk_bindings_update_keymap(NkBindings *self, struct xkb_keymap *keymap, struct xkb_state *state)
{
    g_return_if_fail(self != NULL);

    xkb_keymap_unref(self->keymap);
    xkb_state_unref(self->state);

    self->keymap = xkb_keymap_ref(keymap);
    self->state = xkb_state_ref(state);
}

struct xkb_context *
nk_bindings_get_context(NkBindings *self)
{
    return self->context;
}


static void
_nk_bindings_get_modifiers_masks(NkBindings *self, xkb_keycode_t key, xkb_mod_mask_t *effective, xkb_mod_mask_t *not_consumed)
{
    *effective = 0;
    *not_consumed = 0;

    NkBindingsModifiers mod;
    xkb_mod_index_t *i;
    for ( mod = 0 ; mod < NK_BINDINGS_NUM_MODIFIERS ; ++mod )
    {
        gboolean found = FALSE;
        for ( i = self->modifiers[mod] ; ! found && ( *i != XKB_MOD_INVALID ) ; ++i )
        {
            if ( ! xkb_state_mod_index_is_active(self->state, *i, XKB_STATE_MODS_EFFECTIVE) )
                continue;
            *effective |= (1 << mod);
#ifdef NK_XKBCOMMON_HAS_CONSUMED2
            if ( xkb_state_mod_index_is_consumed2(self->state, key, *i, XKB_CONSUMED_MODE_GTK) != 1 )
#else /* ! NK_XKBCOMMON_HAS_COMSUMED2 */
            if ( xkb_state_mod_index_is_consumed(self->state, key, *i) != 1 )
#endif /* ! NK_XKBCOMMON_HAS_COMSUMED2 */
                *not_consumed |= (1 << mod);

            break;
        }
    }
}

static void
_nk_bindings_free_on_release(NkBindings *self, gboolean trigger)
{
    if ( self->on_release == NULL )
        return;

    GList *link, *next;
    for ( link = self->on_release, next = g_list_next(link) ; link != NULL ; link = next, next = g_list_next(link) )
    {
        NkBindingsBinding *binding = link->data;
        if ( trigger )
            binding->release.base.callback(binding->scope, binding->release.base.user_data);
        binding->release.link = NULL;
        g_list_free_1(link);
    }
    self->on_release = NULL;
}

gchar *
nk_bindings_handle_key(NkBindings *self, xkb_keycode_t keycode, NkBindingsKeyState state)
{
    g_return_val_if_fail(self != NULL, FALSE);

    xkb_keysym_t keysym;
    gchar *tmp = NULL;
    gsize length = 0;

    if ( state == NK_BINDINGS_KEY_STATE_RELEASE )
    {
        xkb_mod_mask_t dummy, mask;
        _nk_bindings_get_modifiers_masks(self, 0, &dummy, &mask);
        if ( mask == 0 )
            _nk_bindings_free_on_release(self, TRUE);
        return NULL;
    }

    keysym = xkb_state_key_get_one_sym(self->state, keycode);

    gboolean regular_press = ( state == NK_BINDINGS_KEY_STATE_PRESS );

#ifdef NK_XKBCOMMON_HAS_COMPOSE
    if ( regular_press && ( self->compose.state != NULL ) && ( keysym != XKB_KEY_NoSymbol ) && ( xkb_compose_state_feed(self->compose.state, keysym) == XKB_COMPOSE_FEED_ACCEPTED ) )
    {
        switch ( xkb_compose_state_get_status(self->compose.state) )
        {
        case XKB_COMPOSE_CANCELLED:
            /* Eat the keysym that cancelled the compose sequence.
             * This is default behaviour with Xlib */
        case XKB_COMPOSE_COMPOSING:
            return NULL;
        case XKB_COMPOSE_COMPOSED:
            keysym = xkb_compose_state_get_one_sym(self->compose.state);
            length = xkb_compose_state_get_utf8(self->compose.state, NULL, 0);
            if ( length != 0 )
            {
                tmp = g_newa(gchar, length + 1);
                length = xkb_compose_state_get_utf8(self->compose.state, tmp, length + 1);
            }
        case XKB_COMPOSE_NOTHING:
        break;
        }
    }
#endif /* NK_XKBCOMMON_HAS_COMPOSE */

    xkb_mod_mask_t effective, not_consumed;
    _nk_bindings_get_modifiers_masks(self, keycode, &effective, &not_consumed);

    if ( _nk_bindings_try_key_bindings(self, effective, not_consumed, keycode, keysym, regular_press) )
        return NULL;

    if ( ! regular_press )
        return NULL;

    if ( length == 0 )
    {
        length = xkb_state_key_get_utf8(self->state, keycode, NULL, 0);
        if ( length != 0 )
        {
            tmp = g_newa(gchar, length + 1);
            length = xkb_state_key_get_utf8(self->state, keycode, tmp, length + 1);
        }
    }

    return g_strndup(tmp, length);
}

gboolean
nk_bindings_handle_button(NkBindings *self, guint button, NkBindingsButtonState state, guint64 timestamp)
{
    g_return_val_if_fail(self != NULL, FALSE);

    xkb_mod_mask_t dummy, mask;

    _nk_bindings_get_modifiers_masks(self, 0, &dummy, &mask);

    if ( state == NK_BINDINGS_BUTTON_STATE_RELEASE )
    {
        if ( mask == 0 )
            _nk_bindings_free_on_release(self, TRUE);
        return TRUE;
    }

    return _nk_bindings_try_button_bindings(self, mask, button, timestamp);
}

void
nk_bindings_update_mask(NkBindings *self, xkb_mod_mask_t depressed_mods, xkb_mod_mask_t latched_mods, xkb_mod_mask_t locked_mods, xkb_layout_index_t depressed_layout, xkb_layout_index_t latched_layout, xkb_layout_index_t locked_layout)
{
    g_return_if_fail(self != NULL);

    enum xkb_state_component changed;

    changed = xkb_state_update_mask(self->state, depressed_mods, latched_mods, locked_mods, depressed_layout, latched_layout, locked_layout);

    if ( changed & XKB_STATE_MODS_EFFECTIVE )
    {
        xkb_mod_mask_t dummy, mask;
        _nk_bindings_get_modifiers_masks(self, 0, &dummy, &mask);
        if ( mask == 0 )
            _nk_bindings_free_on_release(self, TRUE);
    }
}

void
nk_bindings_reset(NkBindings *self)
{
    g_return_if_fail(self != NULL);

    _nk_bindings_free_on_release(self, FALSE);
}
