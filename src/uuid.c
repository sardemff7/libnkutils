/*
 * libnkutils/uuid - Miscellaneous utilities, uuid module
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
#define G_LOG_DOMAIN "libnkutils-uuid"

#include <string.h>

#include <glib.h>

#include "nkutils-uuid.h"
#include "uuid-internal.h"

/**
 * SECTION: nkutils-uuid
 * @title: UUID
 * @short_description: v4 (random) and v5 (namespace) UUID generation
 *
 * An abstraction API for UUID generation. It uses `libuuid` or `apr-util` as available.
 */

/**
 * NkUuid:
 * @data: the UUID has raw bytes
 * @string: the UUID as a string using the xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx format
 *
 * A structure holding the UUID as both raw bytes and string format.
 */

/**
 * NK_UUID_LENGTH:
 *
 * The size in bytes of an UUID.
 */

/**
 * NK_UUID_FORMATTED_LENGTH:
 *
 * The size in characters of the string format of an UUID.
 */

/**
 * NK_UUID_INIT:
 *
 * You can use this macro to initialize an #NkUuid structure:
 * |[<!-- language="C" -->
 * NkUuid uuid = NK_UUID_INIT;
 * ]|
 */

/**
 * nk_uuid_generate:
 * @uuid: (out caller-allocates): an #NkUuid
 *
 * Generates a v4 (random) UUID.
 */

/**
 * nk_uuid_parse:
 * @uuid: (out caller-allocates): an #NkUuid
 * @string: a string containing an UUID in the xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx format
 *
 * Parses an UUID stirng.
 *
 * Returns: %TRUE if parsing succeeded, %FALSE otherwise
 */

#define SHA1_SIZE 20

/**
 * nk_uuid_from_name:
 * @uuid: (inout): an #NkUuid
 * @name: an arbritrary name
 * @length: the length of @name. If @length &lt; 0,
 *
 * Generates a v5 (namespace) UUID.
 *
 * @uuid must contains the namespace UUID and will be updated to the new UUID.
 */
void
nk_uuid_from_name(NkUuid *self, const gchar *name, gssize length)
{
    if ( length < 0 )
        length = strlen(name);

    GChecksum *c;
    c = g_checksum_new(G_CHECKSUM_SHA1);
    g_checksum_update(c, self->data, NK_UUID_LENGTH);
    g_checksum_update(c, (const guchar *) name, length);

    guchar sum[SHA1_SIZE];
    gsize l = SHA1_SIZE;
    g_checksum_get_digest(c, sum, &l);
    g_checksum_free(c);

    memcpy(self->data, sum, NK_UUID_LENGTH);
    /* Set variant as RFC 4122 */
    self->data[8] &= 0x3F;
    self->data[8] |= 0x80;

    /* Set version as 5, SHA-1 hash-based */
    self->data[6] &= 0x0F;
    self->data[6] |= (5 << 4);

    nk_uuid_update_string(self);
}
