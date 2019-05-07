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

#ifndef _HOST_AGENT_CLIENT_H_
#define _HOST_AGENT_CLIENT_H_

#include "host-agent.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize Host Client communication
 *
 * @return the socket fd listen for client connection if > 0, otherwise fail
 */
int
host_agent_client_init();

/**
 * Add a host client
 *
 * @param sock host client socket fd
 *
 * @return 0 if success, otherwise fail
 */
int
host_agent_client_add_client(int sock);

/**
 * Send message to Host Client
 *
 * @parm client the host client
 * @parm buf buffer to be sent
 * @parm len size of the buffer
 *
 * @return success if true, fail if false
 */
bool
host_agent_client_send_msg(const host_client_t *client, const void *buf,
        size_t len);

/**
 * Send coap packet to Host Client
 *
 * @parm client the host client
 * @parm msg the coap packet to be sent
 *
 * @return success if true, fail if false
 */
bool
host_agent_client_send_coap_msg(const host_client_t *client,
        const coap_packet_t *msg);

/**
 * Send request to Host Client
 *
 * @parm client the host client
 * @parm mid message id of the request
 * @parm url url of the request
 * @parm code code of the request
 * @parm payload payload of the request
 * @parm payload_len length of the payload
 *
 * @return success if true, fail if false
 */
bool
host_agent_client_send_request(const host_client_t *client, int mid,
        const char *url, uint8_t code, const uint8_t *payload,
        uint32_t payload_len);

/**
 * Handle message from Host Client
 *
 * @param client the host client
 * @param link_message the imrt link message to be handled
 */
void
host_agent_client_handle_msg(host_client_t *client,
        const imrt_link_message_t *link_message);

/**
 * Wake Host Client
 *
 * @param client the host client
 */
void
host_agent_client_wake_client(host_client_t *client);

/**
 * Free counted buffer
 *
 * @param counted_buf counted buffer
 */
void
host_agent_client_free_counted_buf(counted_buf_t *counted_buf);

/**
 * Free events and transactions that are belong to the client
 *
 * @param client the host client
 */
void
host_agent_client_destroy_client(host_client_t *client);

/**
 * Clear events and transactions that are belong to the client
 *
 * @param client the host client
 */
void
host_agent_client_clear_client(host_client_t *client);

/**
 * Free all clients and its resouces including events and transactions
 */
void
host_agent_client_destroy_clients();

/**
 * Destory all Host Client resources
 */
void
host_agent_client_destroy();

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _HOST_AGENT_CLIENT_H_ */
