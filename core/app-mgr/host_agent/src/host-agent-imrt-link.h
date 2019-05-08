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

#ifndef _HOST_AGENT_IMRT_LINK_H_
#define _HOST_AGENT_IMRT_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Handle one byte data to construct a complete imrt link message
 *
 * @param ch one byte data
 * @param ctx the receive context which contains uncompleted imrt link message
 *
 * @return -1 means invalid sync byte received; 1 means the byte was added to
 * buffer and waiting more for complete packet; 0 completed packet; 2 if in
 * payload reciving
 */
int
on_imrt_link_byte_arrive(unsigned char ch, imrt_link_recv_context_t *ctx);

/**
 * Serialize an imrt link message to buffer
 *
 * @param message the imrt link message
 * @param buffer_out serialized buffer
 *
 * @return buffer length, < 0 means fail
 */
int
imrt_link_message_serialize(const imrt_link_message_t *message,
        char **buffer_out);

/**
 * Serialize a coap packet(over TCP) to imrt link buffer
 *
 * @param coap_packet the coap packet
 * @param buf serialized buffer
 *
 * @return buffer length, < 0 means fail
 */
int
serialize_coap_packet_to_imrt_link(const coap_packet_t *coap_packet,
        char **buf);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _HOST_AGENT_IMRT_LINK_H_ */
