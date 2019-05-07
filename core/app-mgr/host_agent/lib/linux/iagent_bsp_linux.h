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

#ifndef APPS_IAGENT_CORE_IAGENT_BSP_LINUX_H__
#define APPS_IAGENT_CORE_IAGENT_BSP_LINUX_H__

//iagent

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <linux/prctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>

typedef struct _sync {
    pthread_mutex_t condition_mutex;
    pthread_cond_t condition_cond;
    unsigned char locked;
    unsigned char waiting;
} sync_t, *ptr_sync_t;

#endif
