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
 * @file   host-agent-aee.c
 *
 * @brief Handle message from or send message to aee.
 */

#include "host-agent.h"
#include "host-agent-utils.h"
#include "host-agent-log.h"
#include "host-agent-imrt-link.h"
#include "coap_ext.h"
#include "host-agent-client.h"
#include "bh_common.h"

#define DEBUG 0

extern int g_aee_sock;

#if defined (AEE_TCP)
static int listen_socket;
#elif defined (AEE_AON)
#include "libaon.h"
static aon_handle_t aon_handle;
/* the aon poll thread write data to while the
 * main thread read data from this pipe */
static int aon_pipefd[2];
static pthread_t aon_poll_tid = -1;
static bool aon_poll_run = true;
#endif

static void forward_event_to_host_client(const char *event_url,
        const coap_packet_t *coap_packet)
{
    int link_msg_len;
    char *link_msg_buf = NULL;
    extern event_t *g_events;
    event_t *node = g_events;
    int clientCnt = 0;

    LOG_INFO("forward event to host client url:(%s)\n", event_url);

    link_msg_len = serialize_coap_packet_to_imrt_link(coap_packet,
            &link_msg_buf);
    if (link_msg_len <= 0 || link_msg_buf == NULL) {
        LOG_ERROR(
                "forward_event_to_host_client: serialize_coap_packet_to_imrt_link fail %d\n",
                link_msg_len);
        return;
    }

#if DEBUG
    int i = 0;
    for (; i < link_msg_len; i++) {
        printf (" 0x%02x", link_msg_buf[i]);
    }
    printf("\n");
#endif

    counted_buf_t *b = (counted_buf_t *) malloc(sizeof(counted_buf_t));
    if (b == NULL) {
        LOG_ERROR("malloc fail\n");
        free(link_msg_buf);
        return;
    }
    b->buf = link_msg_buf;
    b->len = link_msg_len;
    b->cnt = 1;

    while (node != NULL) {
        if (strcmp(node->url, event_url) == 0) {
            clientCnt++;
            if (!node->disabled) {
                LOG_INFO("forward this event to client sock:%d\n",
                        node->client->sock);
                host_agent_client_send_msg(node->client, link_msg_buf,
                        link_msg_len);
            } else {
                if (node->client->sock == CLIENT_DISCONNECTED) {
                    host_agent_client_wake_client(node->client);
                }

                // buffer event
                event_buf_t *e;
                if (node->list_size >= 32) {
                    // if list size limit is reached, reuse oldest buf to store newest event
                    event_buf_t *pre, *cur = node->event_buf_list;
                    while (cur->next != NULL) {
                        pre = cur;
                        cur = cur->next;
                    }
                    pre->next = NULL;
                    e = cur;
                    host_agent_client_free_counted_buf(e->counted_buf);
                } else {
                    e = (event_buf_t *) malloc(sizeof(event_buf_t));
                    if (e == NULL) {
                        LOG_WARNING("malloc fail\n");
                        continue;
                    }
                    node->list_size++;
                }
                e->counted_buf = b;
                b->cnt++;
                e->next = node->event_buf_list;
                node->event_buf_list = e;
            }
        }
        node = node->next;
    }

    if (clientCnt == 0)
        LOG_INFO("no client has registered this event!\n");

    host_agent_client_free_counted_buf(b);
}

#if defined (AEE_TCP)
bool
host_agent_aee_send_msg(const void *buf, int len)
{
    int cnt = 0;
    ssize_t ret;

    LOG_INFO("host_agent_aee_send_msg begin g_aee_sock:%d buf:%p len:%d\n",
            g_aee_sock, buf, len);

    if (g_aee_sock == -1 || buf == NULL || len <= 0) {
        return false;
    }

    resend:
    ret = send(g_aee_sock, buf, len, 0);

    if (ret == -1) {
        LOG_ERROR("host_agent_aee_send_msg fail with error: %d\n", errno);

        if (errno == ECONNRESET) {
            close(g_aee_sock);
            g_aee_sock = -1;
        }

        // repeat sending if the outbuffer is full
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (++cnt > 10) {
                close(g_aee_sock);
                g_aee_sock = -1;
                return false;
            }
            sleep(1);
            goto resend;
        }
    }

    LOG_INFO("host_agent_aee_send_msg done sent:%d\n", ret);

    return (ret == len);
}
#elif defined (AEE_UART)
bool
host_agent_aee_send_msg(const void *buf, int len)
{
    int ret;
    extern int g_aee_uart;

    LOG_INFO("host_agent_aee_send_msg begin g_aee_uart:%d buf:%p len:%d\n",
            g_aee_uart, buf, len);

    if (g_aee_uart == -1 || buf == NULL || len <= 0) {
        return false;
    }

    ret = write(g_aee_uart, buf, len);

    if (ret == -1) {
        LOG_ERROR("host_agent_aee_send_msg fail with error: %d\n", errno);
        return false;
    }

    LOG_INFO("host_agent_aee_send_msg done sent:%d\n", ret);

    return (ret == len);
}
#elif defined (AEE_HECI)
// TODO
#elif defined (AEE_AON)
bool
host_agent_aee_send_msg(const void *buf, int size)
{
    int sent = 0;
    union {
        struct HostIntfDataBuffer cmd;
        char buf[256];
    }cmd_buf;
    int i, count, max_packet_size;

    LOG_INFO("host_agent_aee_send_msg begin aon_handle:%p buf:%p len:%d\n",
            aon_handle, buf, size);

    if (aon_handle == NULL || buf == NULL || size <= 0) {
        return false;
    }

    cmd_buf.cmd.evtType = EVT_NO_AEE_CONFIG_EVENT;
    max_packet_size = 128;
    count = (size + max_packet_size - 1) / max_packet_size;
    for (i = 0; i < count; i++) {
        int size1 = (i < count - 1) ? max_packet_size : size - max_packet_size * i;
        bh_memcpy_s(cmd_buf.cmd.buffer, max_packet_size, buf + i * max_packet_size, size1);

        if (aon_write(aon_handle, &cmd_buf.cmd, size1 + 4) == 0) /* "4" for Event type */
        sent += size1;
    }

    LOG_INFO("host_agent_aee_send_msg done sent:%d\n", sent);

    return (sent == size);
}
#endif

bool host_agent_aee_send_coap_msg(const coap_packet_t *msg)
{
    int len;
    char *buf;
    bool res;

    if (msg == NULL)
        return false;

    len = serialize_restful_to_imrt_link(msg, &buf);
    if (len < 0) {
        LOG_ERROR(
                "host_agent_aee_send_coap_msg: serialize_coap_packet_to_imrt_link fail %d\n",
                len);
        return false;
    }

    res = host_agent_aee_send_msg(buf, len);

    free(buf);

    return res;
}

bool host_agent_aee_send_request(int mid, const char *url, uint8_t code,
        const uint8_t *payload, uint32_t payload_len)
{
    coap_packet_t packet;

    coap_init_message(&packet, COAP_TYPE_NON, code, mid);
    coap_set_token(&packet, (uint8_t*) &mid, sizeof(mid));
    coap_set_header_content_format(&packet, 0);

    if (url)
        coap_set_header_uri_path(&packet, url);

    if (payload && payload_len > 0)
        coap_set_payload(&packet, payload, payload_len);

    return host_agent_aee_send_coap_msg(&packet);
}

void host_agent_aee_handle_msg(const imrt_link_message_t *link_message)
{
    erbium_status_code = NO_ERROR;
    coap_packet_t message[1];
    extern sync_ctx_t *g_transaction_ctx;

    LOG_INFO("host_agent_aee_handle_msg\n");

    /* so far, no other message type than coap tcp */
    if (link_message->message_type == RESPONSE_PACKET) {
        response_t response[1];
        if (!unpack_response(link_message->payload, link_message->payload_size,
                response))
            return;

        convert_response_to_coap_packet(response, message);

        unsigned long id;

        LOG_VERBOSE("host_agent_aee_handle_msg: handle response\n");

        if (message->token_len != sizeof(long)) {
            LOG_ERROR("host_agent_aee_handle_msg: invalid token len: %d\n",
                    message->token_len);
            return;
        }

        // no need endian convert since the remote service will handle the token as byte array
        id = *((unsigned long *) message->token);
        LOG_VERBOSE("host_agent_aee_handle_msg: id:%d\n", id);
        bh_feed_response(g_transaction_ctx, id, message, 0, T_Coap_Parsed);
    } else if (link_message->message_type == REQUEST_PACKET) {
        request_t request[1];
        if (!unpack_request(link_message->payload, link_message->payload_size,
                request)) {
            LOG_ERROR("failed unpack AEE request\n");
            return;
        }
        convert_request_to_coap_packet(request, message);

        if (message->code == COAP_EVENT) {
            forward_event_to_host_client(request->url, message);
        } else {
            LOG_VERBOSE("unsupported request action (%d) from AEE\n",
                    message->code);
        }
    } else {
        LOG_ERROR("host_agent_aee_handle_msg: unsupported message type %d\n",
                link_message->message_type);
        return;
    }

}

#if defined (AEE_TCP)

int
host_agent_aee_init(const char *p)
{
    uint16_t port = atoi(p);
    listen_socket = listen_tcp_port(port);
    return listen_socket;
}

void
host_agent_aee_destroy()
{
    if (g_aee_sock > 0)
    close(g_aee_sock);

    if (listen_socket > 0)
    close(listen_socket);
}

#elif defined (AEE_UART)

int
host_agent_aee_init(const char *p)
{
    return open_uart(p);
}

void
host_agent_aee_destroy()
{
    extern int g_aee_uart;
    if (g_aee_uart > 0)
    close(g_aee_uart);
}

#elif defined (AEE_AON)

static void*
aon_event_listener(aon_handle_t handle)
{
    struct CommonHostIntfDataBuffer buffer;

    do {
        int ret = aon_poll(handle, &buffer);
        if (ret < 0) {
            LOG_ERROR("aon poll error!\n");
            continue;
        }

        LOG_VERBOSE("Received from aon, event type: 0x%X, total length: %d\n",
                buffer.evtType, ret);

        if (buffer.evtType == EVT_NO_AEE_EVENT) {
            /* write the received data to the write end of the pipe */
            ret = write(aon_pipefd[1], buffer.buffer, ret - 4/*event type */);
            if (ret == -1) {
                LOG_ERROR("write to pipe fail with error: %d\n", errno);
                continue;
            }
            LOG_VERBOSE("Foward %d data to pipe\n", ret);
        }
    }while (aon_poll_run);

    return NULL;
}

int
host_agent_aee_init(const char *p)
{
    int ret;

    aon_pipefd[0] = -1;
    aon_pipefd[1] = -1;

    if (!(aon_handle = aon_open(CHAN_AEE))) {
        LOG_ERROR("aon open fail\n");
        return -1;
    }
    LOG_VERBOSE("aon opened\n");

    if (pipe(aon_pipefd) != 0) {
        LOG_ERROR("create pipe for aon fail %d\n", errno);
        aon_close(aon_handle);
        return -1;
    }
    LOG_VERBOSE("pipe created\n");

    ret = pthread_create(&aon_poll_tid, NULL, aon_event_listener, aon_handle);
    if (ret != 0) {
        LOG_ERROR("create poll thread fro aon fail %d\n", errno);
        close(aon_pipefd[0]);
        close(aon_pipefd[1]);
        aon_close(aon_handle);
        return -1;
    }
    LOG_VERBOSE("aon recv thread created\n");

    /* return the read end of the pipe */
    return aon_pipefd[0];
}

void
host_agent_aee_destroy()
{
    if (aon_poll_tid != -1) {
        aon_poll_run = false;
        pthread_join(aon_poll_tid, NULL);
    }

    if (aon_pipefd[0] != -1)
    close(aon_pipefd[0]);
    if (aon_pipefd[1] != -1)
    close(aon_pipefd[1]);

    if (aon_handle != NULL)
    aon_close(aon_handle);
}

#elif defined (AEE_HECI)
//TODO
#endif

