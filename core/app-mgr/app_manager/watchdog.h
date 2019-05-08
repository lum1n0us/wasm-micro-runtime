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

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include "app-manager.h"

#ifdef __cplusplus
extern "C" {
#endif

bool
watchdog_timer_init(module_data *module_data);

void
watchdog_timer_destroy(watchdog_timer *wd_timer);

void
watchdog_timer_start(watchdog_timer *wd_timer);

void
watchdog_timer_stop(watchdog_timer *wd_timer);

watchdog_timer*
app_manager_get_watchdog_timer(void *timer);

bool
watchdog_startup();

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _WATCHDOG_H_ */
