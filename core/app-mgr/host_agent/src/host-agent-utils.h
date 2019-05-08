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

#ifndef _HOST_AGENT_UTILS_H_
#define _HOST_AGENT_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open and listen on a TCP port
 *
 * @param port the port
 *
 * @return the socket fd, fail if < 0
 */
int
listen_tcp_port(uint16_t port);

/**
 * Open uart device
 *
 * @param uart_dev the device name
 *
 * @return the uart fd, fail if < 0
 */
int
open_uart(const char *uart_dev);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _HOST_AGENT_UTILS_H_ */
