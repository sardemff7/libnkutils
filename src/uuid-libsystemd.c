/*
 * libnkutils/uuid - Miscellaneous utilities, uuid module
 *
 * Copyright © 2011-2021 Quentin "Sardem FF7" Glidic
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

#include <glib.h>

#include <systemd/sd-id128.h>

#include "nkutils-uuid.h"
#include "uuid-internal.h"

NK_EXPORT void
nk_uuid_get_machine_app_specific(NkUuid *uuid, NkUuid app_uuid)
{
    sd_id128_t id;
    sd_id128_t ret_id;
    memcpy(&id.bytes, app_uuid.data, NK_UUID_LENGTH);
    sd_id128_get_machine_app_specific(id, &ret_id);
    memcpy(uuid->data, ret_id.bytes, NK_UUID_LENGTH);
    nk_uuid_update_string(uuid);
}
