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

#ifndef _EVENT_H_
#define _EVENT_H_

#include "bh_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Handle event request from host agent
 *
 * @param code the coap packet code
 * @param event_url the event url
 *
 * @return true if success, false otherwise
 */
bool
event_handle_event_request(uint8_t code, const char *event_url,
        uint32_t register);

/**
 * Test whether the event is registered
 *
 * @param event_url the event url
 *
 * @return true for registered, false for not registered
 */
bool
event_is_registered(const char *event_url);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _EVENT_H_ */
