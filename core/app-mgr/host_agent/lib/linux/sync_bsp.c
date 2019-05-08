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

#define _GNU_SOURCE

#include <stdlib.h>
#include <errno.h>
#include "iagent_bsp.h"

void bh_bsp_lock(ptr_sync_t sync_obj)
{
    pthread_mutex_lock(&sync_obj->condition_mutex);
    sync_obj->locked = 1;
}

void bh_bsp_unlock(ptr_sync_t sync_obj)
{
    sync_obj->locked = 0;
    pthread_mutex_unlock(&sync_obj->condition_mutex);
}

time_t bh_bsp_get_time()
{
    time_t t;
    time(&t);
    return t;
}

// 返回自系统开机以来的毫秒数（tick）
tick_time_t bh_get_tick_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// 返回自系统开机以来的秒数（tick）
ptr_sync_t bh_bsp_create_syncobj()
{
    ptr_sync_t sync_obj = (ptr_sync_t) malloc(sizeof(sync_t));
    if (sync_obj == NULL)
        return NULL;

    memset(sync_obj, 0, sizeof(*sync_obj));

    pthread_mutex_init(&sync_obj->condition_mutex, NULL); //PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_init(&sync_obj->condition_cond, NULL); // = PTHREAD_COND_INITIALIZER;

    return sync_obj;

}

void bh_bsp_delete_syncobj(ptr_sync_t sync_obj)
{
    if (sync_obj->locked)
        bh_bsp_unlock(sync_obj);

    pthread_mutex_destroy(&sync_obj->condition_mutex);
    pthread_cond_destroy(&sync_obj->condition_cond);
    free(sync_obj);
}

int bh_bsp_wait(ptr_sync_t sync_obj, int timeout_ms, bool hold_lock)
{
    struct timespec abstime;

    struct timeval now;

//    if(sync_obj->locked == 0)
//        hb_bsp_lock(sync_obj);

    gettimeofday(&now, NULL);

    int nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;

    abstime.tv_nsec = nsec % 1000000000;

    abstime.tv_sec = now.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
    sync_obj->waiting = 1;
    int ret = pthread_cond_timedwait(&sync_obj->condition_cond,
            &sync_obj->condition_mutex, &abstime);
    sync_obj->waiting = 0;

    if (!hold_lock)
        bh_bsp_unlock(sync_obj);

    if (ETIMEDOUT == ret)
        return -1;
    else
        return 0;

}

void bh_bsp_wakeup(ptr_sync_t sync_obj, bool hold_lock)
//    if(sync_obj->locked == 0)
//        hb_bsp_lock(sync_obj);
{
    pthread_cond_signal(&sync_obj->condition_cond);

    if (!hold_lock)
        bh_bsp_unlock(sync_obj);

}

