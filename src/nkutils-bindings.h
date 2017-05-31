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

#ifndef __NK_UTILS_BINDINGS_H__
#define __NK_UTILS_BINDINGS_H__

#include <xkbcommon/xkbcommon.h>

typedef struct _NkBindings NkBindings;

typedef enum {
    NK_BINDINGS_MODIFIER_SHIFT,
    NK_BINDINGS_MODIFIER_CONTROL,
    NK_BINDINGS_MODIFIER_ALT,
    NK_BINDINGS_MODIFIER_SUPER,
    NK_BINDINGS_MODIFIER_META,
    NK_BINDINGS_MODIFIER_HYPER,
#define NK_BINDINGS_NUM_MODIFIERS 6
} NkBindingsModifiers;

typedef enum {
    NK_BINDINGS_KEY_STATE_PRESS,
    NK_BINDINGS_KEY_STATE_PRESSED,
    NK_BINDINGS_KEY_STATE_RELEASE,
} NkBindingsKeyState;

typedef enum {
    NK_BINDINGS_BUTTON_STATE_PRESS,
    NK_BINDINGS_BUTTON_STATE_RELEASE,
} NkBindingsButtonState;

#define NK_BINDINGS_MODIFIER_MASK(m) (1 << (m))

typedef enum {
    NK_BINDINGS_ERROR_SYNTAX,
    NK_BINDINGS_ERROR_NOTHING,
    NK_BINDINGS_ERROR_ALREADY_REGISTERED,
} NkBindingsError;
#define NK_BINDINGS_ERROR (nk_bindings_error())
GQuark nk_bindings_error(void);

NkBindings *nk_bindings_new(struct xkb_context *context, struct xkb_keymap *keymap, struct xkb_state *state);
void nk_bindings_free(NkBindings *bindings);

typedef gboolean (*NkBindingsCallback)(guint scope, gpointer user_data);
gboolean nk_bindings_add_binding(NkBindings *bindings, guint scope, const gchar *string, NkBindingsCallback callback, gpointer user_data, GDestroyNotify notify, GError **error);


struct xkb_context *nk_bindings_get_context(NkBindings *bindings);

gchar *nk_bindings_handle_key(NkBindings *bindings, xkb_keycode_t key, NkBindingsKeyState state);
gboolean nk_bindings_handle_button(NkBindings *bindings, guint button, NkBindingsButtonState state, guint64 timestamp);
void nk_bindings_update_keymap(NkBindings *self, struct xkb_keymap *keymap, struct xkb_state *state);
void nk_bindings_update_mask(NkBindings *bindings, xkb_mod_mask_t depressed_mods, xkb_mod_mask_t latched_mods, xkb_mod_mask_t locked_mods, xkb_layout_index_t depressed_layout, xkb_layout_index_t latched_layout, xkb_layout_index_t locked_layout);
void nk_bindings_reset(NkBindings *bindings);

#endif /* __NK_UTILS_BINDINGS_H__ */
