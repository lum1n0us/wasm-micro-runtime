/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2018 Intel Corporation
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

#ifndef LIB_IAGENT_BSP_H_
#define LIB_IAGENT_BSP_H_

#if defined(RUN_ON_LINUX) | 1
#include "linux/iagent_bsp_linux.h"
#elif defined (RUN_ON_VXWORKS)

#elif defined (RUN_ON_ZEPHRY)

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long tick_time_t;

time_t bh_bsp_get_time();
ptr_sync_t bh_bsp_create_syncobj();
void bh_bsp_delete_syncobj(ptr_sync_t);
void bh_bsp_lock(ptr_sync_t);
void bh_bsp_unlock(ptr_sync_t);
int bh_bsp_wait(ptr_sync_t sync_obj, int timeout_ms, bool hold_lock);
void bh_bsp_wakeup(ptr_sync_t, bool hold_lock);
uint32_t bh_get_elpased_ms(uint32_t * last_system_clock);
tick_time_t bh_get_tick_ms();
tick_time_t bh_get_tick_sec();

#ifdef __cplusplus
}
#endif
#endif /* LIB_IAGENT_BSP_H_ */
