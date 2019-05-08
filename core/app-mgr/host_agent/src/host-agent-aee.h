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

#ifndef _HOST_AGENT_AEE_H_
#define _HOST_AGENT_AEE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize communication with AEE
 *
 * @param param
 *
 * @return the file discriptor(listen socket for TCP mode or uart
 * fd for UART mode). < 0 means fail
 */
int
host_agent_aee_init(const char *param);

/**
 * Destroy aee related resources
 */
void
host_agent_aee_destroy();

/**
 * Send message to AEE
 *
 * @param buf buffer to send
 * @param len size of buffer
 *
 * @return true for success, false for fail
 */
bool
host_agent_aee_send_msg(const void *buf, int len);

/**
 * Send coap packet to AEE
 *
 * @parm msg the coap packet to send
 *
 * @return true for success, false for fail
 */
bool
host_agent_aee_send_coap_msg(const coap_packet_t *msg);

/**
 * Send request to AEE
 *
 * @parm mid message id of the request
 * @parm url url of the request
 * @parm code code of the request
 * @parm payload payload of the request
 * @parm payload_len length of the payload
 *
 * @return true for success, false for fail
 */
bool
host_agent_aee_send_request(int mid, const char *url, uint8_t code,
        const uint8_t *payload, uint32_t payload_len);

/**
 * Handle message from AEE
 *
 * @param link_message the imrt link message to be handled
 */
void
host_agent_aee_handle_msg(const imrt_link_message_t *link_message);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _HOST_AGENT_AEE_H_ */
