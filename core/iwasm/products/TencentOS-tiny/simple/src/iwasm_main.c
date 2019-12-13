/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdlib.h>
#include <string.h>
#include "bh_platform.h"
#include "wasm_assert.h"
#include "wasm_log.h"
#include "wasm_platform_log.h"
#include "wasm_thread.h"
#include "wasm_export.h"
#include "wasm_memory.h"
#include "bh_memory.h"
#include "test_wasm.h"
#include "cmsis_os.h"

static int app_argc;
static char **app_argv;

static void*
app_instance_main(wasm_module_inst_t module_inst)
{
    const char *exception;

    wasm_application_execute_main(module_inst, app_argc, app_argv);
    if ((exception = wasm_runtime_get_exception(module_inst)))
        wasm_printf("%s\n", exception);
    return NULL;
}

static char global_heap_buf[128 * 1024] = { 0 };

void iwasm_main(void *arg1)
{
    uint8 *wasm_file_buf = NULL;
    int wasm_file_size;
    wasm_module_t wasm_module = NULL;
    wasm_module_inst_t wasm_module_inst = NULL;
    char error_buf[128];
#if WASM_ENABLE_LOG != 0
    int log_verbose_level = 1;
#endif

    (void) arg1;

    wasm_printf("iwasm_main\n");

    if (bh_memory_init_with_pool(global_heap_buf, sizeof(global_heap_buf))
            != 0) {
        wasm_printf("Init global heap failed.\n");
        return;
    }

    /* initialize runtime environment */
    if (!wasm_runtime_init())
        goto fail1;

    wasm_printf("runtime_init sucess \n");

#if WASM_ENABLE_LOG != 0
    wasm_log_set_verbose_level(log_verbose_level);
#endif

    /* load WASM byte buffer from byte buffer of include file */
    wasm_file_buf = (uint8*) wasm_test_file;
    wasm_file_size = sizeof(wasm_test_file);

    /* load WASM module */
    if (!(wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                          error_buf, sizeof(error_buf)))) {
        wasm_printf("%s\n", error_buf);
        goto fail2;
    }

    wasm_printf("rutime_load_success\n\n");

    /* instantiate the module */
    if (!(wasm_module_inst = wasm_runtime_instantiate(wasm_module,
                                                      8 * 1024,
                                                      8 * 1024,
                                                      error_buf,
                                                      sizeof(error_buf)))) {
        wasm_printf("%s\n", error_buf);
        goto fail3;
    }

    wasm_printf("rutime_instantiate_success\n\n");
    app_instance_main(wasm_module_inst);

    /* destroy the module instance */
    wasm_runtime_deinstantiate(wasm_module_inst);

fail3:
    /* unload the module */
    wasm_runtime_unload(wasm_module);

fail2:
    /* destroy runtime environment */
    wasm_runtime_destroy();

fail1:
    bh_memory_destroy();
}

#define IWASM_STK_SIZE          6*1024
extern void iwasm_main(void *arg);
osThreadDef(iwasm_main, osPriorityNormal, 1, IWASM_STK_SIZE);

void application_entry(void *arg)
{
    osThreadCreate(osThread(iwasm_main), NULL); // Create task for iwasm
}
