/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include "transport.h"

#define SA struct sockaddr

unsigned char leading[2] = { 0x12, 0x34 };

bool tcp_init(const char *address, uint16_t port, int *fd)
{
    int sock;
    struct sockaddr_in servaddr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return false;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(address);
    servaddr.sin_port = htons(port);

    if (connect(sock, (SA*) &servaddr, sizeof(servaddr)) != 0)
        return false;

    *fd = sock;
    return true;
}

bool uart_init(const char *conn_str, int *fd)
{
    return false;
}

bool udp_send(const char *address, int port, const char *buf, int len)
{
    //TODO;
    return false;
}

bool host_tool_send_data(int fd, const char *buf, unsigned int len)
{
    int cnt = 0;
    ssize_t ret;

    if (fd == -1 || buf == NULL || len <= 0) {
        return false;
    }

    resend: ret = send(fd, buf, len, 0);

    if (ret == -1) {
        if (errno == ECONNRESET) {
            close(fd);
        }

        // repeat sending if the outbuffer is full
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (++cnt > 10) {
                close(fd);
                return false;
            }
            sleep(1);
            goto resend;
        }
    }

    return (ret == len);
}

#define SET_RECV_PHASE(ctx, new_phase) {ctx->phase = new_phase; ctx->size_in_phase = 0;}

/*
 * input:    1 byte from remote
 * output:   parse result
 * return:   -1 invalid sync byte
 *           1 byte added to buffer, waiting more for complete packet
 *           0 completed packet
 *           2 in receiving payload
 */
int on_imrt_link_byte_arrive(unsigned char ch, imrt_link_recv_context_t *ctx)
{
    if (ctx->phase == Phase_Non_Start) {
        if (ctx->message.payload) {
            free(ctx->message.payload);
            ctx->message.payload = NULL;
            ctx->message.payload_size = 0;
        }

        if (leading[0] == ch) {
            ctx->phase = Phase_Leading;
        } else {
            return -1;
        }
    } else if (ctx->phase == Phase_Leading) {
        if (leading[1] == ch) {
            SET_RECV_PHASE(ctx, Phase_Type);
        } else {
            ctx->phase = Phase_Non_Start;
            return -1;
        }
    } else if (ctx->phase == Phase_Type) {
        unsigned char *p = (unsigned char *) &ctx->message.message_type;
        p[ctx->size_in_phase++] = ch;

        if (ctx->size_in_phase == sizeof(ctx->message.message_type)) {
            ctx->message.message_type = ntohs(ctx->message.message_type);
            SET_RECV_PHASE(ctx, Phase_Size);
        }
    } else if (ctx->phase == Phase_Size) {
        unsigned char * p = (unsigned char *) &ctx->message.payload_size;
        p[ctx->size_in_phase++] = ch;

        if (ctx->size_in_phase == sizeof(ctx->message.payload_size)) {
            ctx->message.payload_size = ntohl(ctx->message.payload_size);
            SET_RECV_PHASE(ctx, Phase_Payload);

            if (ctx->message.payload) {
                free(ctx->message.payload);
                ctx->message.payload = NULL;
            }

            /* no payload */
            if (ctx->message.payload_size == 0) {
                SET_RECV_PHASE(ctx, Phase_Non_Start);
                return 0;
            }

            if (ctx->message.payload_size > 1024 * 1024) {
                SET_RECV_PHASE(ctx, Phase_Non_Start);
                return -1;
            }

            ctx->message.payload = (char *) malloc(ctx->message.payload_size);
            SET_RECV_PHASE(ctx, Phase_Payload);
        }
    } else if (ctx->phase == Phase_Payload) {
        ctx->message.payload[ctx->size_in_phase++] = ch;

        if (ctx->size_in_phase == ctx->message.payload_size) {
            SET_RECV_PHASE(ctx, Phase_Non_Start);
            return 0;
        }

        return 2;
    }

    return 1;
}
