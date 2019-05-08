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
/**
 * @file   host-agent-utils.c
 *
 * @brief Utilities.
 */

#include "host-agent.h"
#include "host-agent-log.h"

#include <termios.h>

#define UART_BAUT_RATE B115200

int listen_tcp_port(uint16_t port)
{
    int sock = -1;
    struct sockaddr_in name;
    struct linger ling;

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_ERROR("create socket fail with error %d\n", errno);
        return -1;
    }

    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    ling.l_onoff = 1;
    ling.l_linger = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *) &ling,
            sizeof(ling)) == -1) {
        LOG_ERROR("sesockopt fail with error %d\n", errno);
        goto fail;
    }

    if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
        LOG_ERROR("bind port %d fail with error %d\n", port, errno);
        goto fail;
    }

    if (listen(sock, 1) < 0) {
        LOG_ERROR("listen sock %d port %d fail with error %d\n", sock, port,
                errno);
        goto fail;
    }

    return sock;

    fail: close(sock);
    return -1;
}

int open_uart(const char *uart_dev)
{
    struct termios uart_term;
    int fd;

    LOG_VERBOSE("Open Uart device %s ...\n", uart_dev);

    fd = open(uart_dev, O_RDWR | O_NOCTTY);

    if (fd <= 0) {
        LOG_VERBOSE("Open Uart device failed.\n");
        return -1;
    }
    LOG_VERBOSE("uart fd: %d\n", fd);

    memset(&uart_term, 0, sizeof(uart_term));
    uart_term.c_cflag = UART_BAUT_RATE | CS8 | CLOCAL | CREAD;
    uart_term.c_iflag = IGNPAR;
    uart_term.c_oflag = 0;

    /* set noncanonical mode */
    uart_term.c_lflag = 0;
    uart_term.c_cc[VTIME] = 30;
    uart_term.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &uart_term) != 0) {
        LOG_VERBOSE("Open Uart device failed: set attr failed.\n");
        close(fd);
        return -1;
    }

    LOG_INFO("Open Uart device success!\n");
    return fd;
}
