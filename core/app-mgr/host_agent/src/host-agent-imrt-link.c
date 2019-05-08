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
 * @file   host-agent-imrt-link.c
 *
 * @brief IMRT link message process.
 */

#include "host-agent.h"
#include "host-agent-log.h"
#include "coap_ext.h"
#include "bh_common.h"
#include "native_interface.h"

#define SET_RECV_PHASE(ctx, new_phase) {ctx->phase = new_phase; ctx->size_in_phase = 0;}
unsigned char leading[2] = { 0x12, 0x34 };

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
                LOG_WARNING("payload size over max: %d\n",
                        ctx->message.payload_size);
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

/*
 * Serialize an imrt_link_message into buffer
 *
 * @return size of the buffer
 */
int imrt_link_message_serialize(const imrt_link_message_t *message,
        uint8_t **buffer_out)
{
    int total;
    uint8_t *p;
    uint16_t type;
    uint32_t size;

    if (NULL == message || NULL == buffer_out)
        return -1;

    total = 2 + 2 + 4 + message->payload_size; /* 2:leading + 2:type + 4:size + payload */

    if (NULL == (*buffer_out = (uint8_t *) malloc(total))) {
        LOG_FATAL("imrt_link_message_serialize malloc fail\n");
        return -1;
    }

    p = *buffer_out;

    /* leading bytes */
    bh_memcpy_s(p, 2, leading, 2);
    p += 2;

    /* message type */
    unsigned short * v = (unsigned short *) (p);
    *v = htons(message->message_type);
    p += 2;

    /* payload size */
    *((unsigned long *) p) = htonl(message->payload_size);
    p += 4;

    /* payload */
    bh_memcpy_s(p, message->payload_size, message->payload,
            message->payload_size);

    LOG_VERBOSE("imrt_link_message_serialize done type:%d payload_size:%d\n",
            message->message_type, message->payload_size);

    return total;
}

int serialize_coap_packet_to_imrt_link(const coap_packet_t *coap_packet,
        uint8_t **buf)
{
    imrt_link_message_t link_message[1] = { 0 };
    char *coap_buf = NULL;
    int coap_len, link_msg_len;

    coap_len = coap_serialize_message_tcp((void *) coap_packet,
            (uint8_t **) &coap_buf);
    if (coap_len <= 0 || coap_buf == NULL) {
        LOG_ERROR(
                "prepare_imrt_link_message_buf: coap_serialize_message_tcp fail %d\n",
                coap_len);
        return -1;
    }

    memset(link_message, 0, sizeof(imrt_link_message_t));
    link_message->message_type = COAP_TCP_RAW;
    link_message->payload = coap_buf;
    link_message->payload_size = coap_len;

    link_msg_len = imrt_link_message_serialize(link_message, buf);
    if (link_msg_len <= 0 || buf == NULL) {
        LOG_ERROR(
                "prepare_imrt_link_message_buf: imrt_link_message_serialize fail %d\n",
                link_msg_len);
        free(coap_buf);
        return -1;
    }

    free(coap_buf);
    return link_msg_len;
}

/* Package Type */
typedef enum {
    Wasm_Bytecode = 0, Wasm_AoT, Package_Type_Unknown = 0xFFFF
} package_type_t;

static package_type_t get_package_type(const char *buf, int size)
{
    if (buf && size > 4) {
        if (buf[0] == '\0' && buf[1] == 'a' && buf[2] == 's' && buf[3] == 'm')
            return Wasm_Bytecode;
        if (buf[0] == '\0' && buf[1] == 'a' && buf[2] == 'o' && buf[3] == 't')
            return Wasm_AoT;
    }
    return Package_Type_Unknown;
}

int serialize_restful_to_imrt_link(const coap_packet_t *coap_packet,
        uint8_t **buf)
{
    imrt_link_message_t link_message[1] = { 0 };
    int len = 0;
    int link_msg_len;
    char * payload = NULL;

    memset(link_message, 0, sizeof(imrt_link_message_t));
    if (coap_packet->code <= COAP_EVENT) {
        request_t request[1];
        int properties_offset;
        int url_len = convert_coap_packet_to_request(coap_packet, request);

        //ensure url end with 0
        request->url = coap_get_full_url_alloc(coap_packet);

        link_message->message_type = REQUEST_PACKET;

        if ((properties_offset = check_url_start(request->url,
                strlen(request->url), "/applet")) > 0
                && request->action == COAP_PUT) {
            char type[16];
            char *properties = request->url + properties_offset;
            find_key_value(properties, strlen(properties), "type", type,
                    sizeof(type) - 1, '&');
            if ((strlen(type) == 0 || strcmp(type, "wasm") == 0)
                    && get_package_type(request->payload, request->payload_len)
                            == Wasm_Bytecode)
                link_message->message_type = INSTALL_WASM_BYTECODE_APP;
        }

        payload = pack_request(request, &len);
        bh_free(request->url);

    } else {
        link_message->message_type = RESPONSE_PACKET;

        // todo: implement
    }
    link_message->payload = payload;
    link_message->payload_size = len;

    link_msg_len = imrt_link_message_serialize(link_message, buf);

    if (payload)
        free_req_resp_packet(payload);

    if (link_msg_len <= 0 || buf == NULL) {
        LOG_ERROR(
                "prepare_imrt_link_message_buf: imrt_link_message_serialize fail %d\n",
                link_msg_len);
        return -1;
    }
    return link_msg_len;
}
