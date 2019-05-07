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

#ifndef _MODULE_JEFF_H_
#define _MODULE_JEFF_H_

#include "app-manager.h"

#ifdef __cplusplus
extern "C" {
#endif

extern module_interface jeff_module_interface;

/* sensor event */
typedef struct bh_sensor_event_t {
    /* Java sensor object */
    void *sensor;
    /* event of attribute container from context core */
    void *event;
} bh_sensor_event_t;

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _MODULE_JEFF_H_ */
