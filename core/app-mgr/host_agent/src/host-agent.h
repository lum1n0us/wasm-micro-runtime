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

#ifndef SRC_HOST_AGENT_H_
#define SRC_HOST_AGENT_H_
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>

#include "agent_core_lib.h"

#include "er-coap-constants.h"
#include "er-coap.h"
#include "host_link.h"

typedef struct {
    unsigned short message_type;
    unsigned long payload_size;
    char *payload;
} imrt_link_message_t;

typedef enum {
    Phase_Non_Start, Phase_Leading, Phase_Type, Phase_Size, Phase_Payload
} recv_phase_t;

typedef struct {
    recv_phase_t phase;
    int size_in_phase;
    imrt_link_message_t message;
} imrt_link_recv_context_t;

#define DAEMON_PACKAGE "com.intel.aee.daemon"

// reuse sock in host_client_t to store the status
#define CLIENT_DISCONNECTED -1
#define CLIENT_WAKED        -2

typedef struct _host_client {
    struct _host_client *next;
    char *package_name; // identifier of android app
    int sock;
    bool first_request_arrived;
    time_t wake_timestamp;
    imrt_link_recv_context_t recv_ctx;
} host_client_t;

typedef struct {
    unsigned char token[4];
    const host_client_t *client;
} aee_trans_ctx_t;

typedef struct {
    void *buf;
    size_t len;
    int cnt;
} counted_buf_t;

typedef struct _event_buf {
    struct _event_buf *next;
    counted_buf_t *counted_buf;
} event_buf_t;

typedef struct _event {
    struct _event *next;
    host_client_t *client; /* client who has registered this event */
    bool disabled;
    event_buf_t *event_buf_list; /* event buffer list for disabled client */
    int list_size;
    char url[1]; /* event url */
} event_t;

#endif /* SRC_HOST_AGENT_H_ */
