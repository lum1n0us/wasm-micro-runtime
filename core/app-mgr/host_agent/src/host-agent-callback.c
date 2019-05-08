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
 * @file   host-agent-callback.c
 *
 * @brief Transaction callbacks.
 */

#include "host-agent.h"
#include "host-agent-log.h"
#include "host-agent-imrt-link.h"
#include "host-agent-client.h"

int cb_foward_aee_response_to_client(void *ctx_data, void *data, int len,
        unsigned char format)
{
    aee_trans_ctx_t *ctx = (aee_trans_ctx_t *) ctx_data;
    char *link_msg_buf = NULL;
    size_t link_msg_len;
    int result = -1;

    LOG_INFO(
            "cb_foward_aee_response_to_client begin data:%p len:%d format:%d mid:%d sock:%d\n",
            data, len, format, ntohl(*(int * )(ctx->token)), ctx->client->sock);

    if (NULL == data) {
        LOG_VERBOSE("aee response TIMEOUT\n");
        // maybe to send a error response for timeout
    } else {
        assert(format == T_Coap_Parsed);
        coap_packet_t *coap_packet = (coap_packet_t *) data;

        coap_set_token(coap_packet, ctx->token, 4);

        link_msg_len = serialize_coap_packet_to_imrt_link(coap_packet,
                &link_msg_buf);

        if (link_msg_len <= 0 || link_msg_buf == NULL) {
            LOG_ERROR(
                    "cb_foward_aee_response_to_client: serialize_coap_packet_to_imrt_link fail %d\n",
                    link_msg_len);
            goto ret;
        }

        if (!host_agent_client_send_msg(ctx->client, link_msg_buf,
                link_msg_len)) {
            LOG_ERROR(
                    "cb_foward_aee_response_to_client host_agent_client_send_msg fail\n");
            // TODO: do we need to stop select on this sock ?
            goto ret;
        }
    }

    result = 0;

    ret: if (link_msg_buf)
        free(link_msg_buf);

    if (ctx_data)
        free(ctx_data);

    LOG_INFO("cb_foward_aee_response_to_client done\n");

    return result;
}
