/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2019 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (License). Unless the License provides otherwise, you
 * may not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */

#include "module_wasm_lib.h"

static bool wasm_lib_module_init(void)
{
    return false;
}

static bool wasm_lib_module_install(request_t *msg)
{
    (void) msg;
    return false;
}

static bool wasm_lib_module_uninstall(request_t *msg)
{
    (void) msg;
    return false;
}

static void wasm_lib_module_watchdog_kill(module_data *m_data)
{
    (void) m_data;
}

static bool wasm_lib_module_handle_host_url(void *queue_msg)
{
    (void) queue_msg;
    return false;
}

static module_data*
wasm_lib_module_get_module_data(void)
{
    return NULL;
}

module_interface wasm_lib_module_interface = { wasm_lib_module_init,
        wasm_lib_module_install, wasm_lib_module_uninstall,
        wasm_lib_module_watchdog_kill, wasm_lib_module_handle_host_url,
        wasm_lib_module_get_module_data,
        NULL };

