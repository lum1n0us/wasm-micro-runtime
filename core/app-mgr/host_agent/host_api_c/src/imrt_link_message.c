#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "er-coap.h"
#include "imrt_link_message.h"
#define app_manager_printf printf
#define bh_free free
#define bh_malloc malloc

#pragma pack(1)
typedef struct imrt_link_message_t {
    unsigned short message_type;
    unsigned int payload_size; //previously is long
    char *payload;
} imrt_link_message;

int imrt_link_message_encode(char *input, int in_len, char **output,
        int *out_len)
{
    int total_len = 2 + 2 + 4 + in_len;
    char *buf = malloc(total_len);
    imrt_link_message *msg = (imrt_link_message *) (buf + 2);
    buf[0] = 0x12;
    buf[1] = 0x34;
    msg->message_type = 0;
    msg->payload_size = htonl(in_len);
    memcpy(&msg->payload, input, in_len);
    *output = buf;
    *out_len = total_len;
}
static bool enable_log = true;

/* Two leading bytes */
static unsigned char leadings[] = { (unsigned char) 0x12, (unsigned char) 0x34 };

static recv_context_t recv_ctx;

static int on_imrt_link_byte_arrive(unsigned char ch, recv_context_t *ctx)
{
    if (ctx->phase == Phase_Non_Start) {
        ctx->message.payload_size = 0;

        if (ctx->message.payload) {
            bh_free(ctx->message.payload);
            ctx->message.payload = NULL;
        }

        if (ch == leadings[0]) {
            if (enable_log)
                app_manager_printf("##On byte arrive: got leading 0\n");
            ctx->phase = Phase_Leading;
        }

        return 0;
    } else if (ctx->phase == Phase_Leading) {
        if (ch == leadings[1]) {
            if (enable_log)
                app_manager_printf("##On byte arrive: got leading 1\n");
            ctx->phase = Phase_Type;
        } else
            ctx->phase = Phase_Non_Start;

        return 0;
    } else if (ctx->phase == Phase_Type) {
        if (ctx->size_in_phase++ == 0) {
            if (enable_log)
                app_manager_printf("##On byte arrive: got type 0\n");
            ctx->message.message_type = ch;
        } else {
            if (enable_log)
                app_manager_printf("##On byte arrive: got type 1\n");
            ctx->message.message_type |= (ch << 8);
            ctx->phase = Phase_Size;
            ctx->size_in_phase = 0;
        }

        return 0;
    } else if (ctx->phase == Phase_Size) {
        unsigned char *p = (unsigned char *) &ctx->message.payload_size;

        if (enable_log)
            app_manager_printf("##On byte arrive: got payload_size, byte %d\n",
                    ctx->size_in_phase);
        p[ctx->size_in_phase++] = ch;

        if (ctx->size_in_phase == sizeof(ctx->message.payload_size)) {
#ifndef __ZEPHYR__
            ctx->message.payload_size = ntohl(ctx->message.payload_size);
#else
            if (is_little_endian())
            exchange32((uint8_t *)&ctx->message.payload_size);
#endif
            ctx->phase = Phase_Payload;

            if (enable_log)
                app_manager_printf("##On byte arrive: payload_size: %d\n",
                        ctx->message.payload_size);
            if (ctx->message.payload) {
                bh_free(ctx->message.payload);
                ctx->message.payload = NULL;
            }

            /* message completion */
            if (ctx->message.payload_size == 0) {
                ctx->phase = Phase_Non_Start;
                if (enable_log)
                    app_manager_printf(
                            "##On byte arrive: receive end, payload_size is 0.\n");
                return 1;
            }

            if (ctx->message.payload_size > 1024 * 1024) {
                ctx->phase = Phase_Non_Start;
                return 0;
            }

            ctx->message.payload = (char *) bh_malloc(
                    ctx->message.payload_size);
            if (!ctx->message.payload) {
                ctx->phase = Phase_Non_Start;
                return 0;
            }

            ctx->phase = Phase_Payload;
            ctx->size_in_phase = 0;
        }

        return 0;
    } else if (ctx->phase == Phase_Payload) {
        ctx->message.payload[ctx->size_in_phase++] = ch;

        if (ctx->size_in_phase == ctx->message.payload_size) {
            ctx->phase = Phase_Non_Start;
            if (enable_log)
                app_manager_printf(
                        "##On byte arrive: receive end, payload_size is %d.\n",
                        ctx->message.payload_size);
            return 1;
        }

        return 0;
    }

    return 0;
}

int aee_host_msg_callback(void *msg, uint16_t msg_len)
{
    unsigned char *p = msg, *p_end = p + msg_len;
    int i;
    /*app_manager_printf("App Manager receive %d bytes from Host\n", msg_len);*/

    while (p < p_end) {
        if (on_imrt_link_byte_arrive(*p, &recv_ctx) == 1) {
            int length = sizeof(bh_queue_msg_t) + sizeof(coap_packet_t);
            bh_queue_msg_t *msg;
            char *payload1;
            if ((msg = bh_malloc(length))) {
                msg->payload = (char *) msg + sizeof(bh_queue_msg_t);
                app_manager_printf("coap_parse_message_tcp\n");
                coap_parse_message_tcp(msg->payload,
                        (uint8_t *) recv_ctx.message.payload,
                        recv_ctx.message.payload_size);
                msg->message_type = COAP_PARSED;
                msg->payload_size = sizeof(coap_packet_t);
                payload1 = (uint8_t *) recv_ctx.message.payload;

                for (i = 0; i < recv_ctx.message.payload_size; i++) {
                    app_manager_printf("%2x ", payload1[i]);
                }
                app_manager_printf("\n");
                for (i = 0; i < recv_ctx.message.payload_size; i++) {
                    app_manager_printf("%c ", payload1[i]);
                }
                app_manager_printf("\n");
#if 0
                /* TODO: use RESPONSE_PACKET from core/app-mgr/app-mgr-shared/host_link.h instead of 3 */
                if (recv_ctx.message.message_type == /*RESPONSE_PACKET*/3)
                {
                    handle_response(msg);
                }
                /* TODO: use REQUEST_PACKET from core/app-mgr/app-mgr-shared/host_link.h instead of 2 */
                else if (recv_ctx.message.message_type == /*REQUEST_PACKET*/2)
                {
                    coap_packet_t *packet = (coap_packet_t *)msg->payload;
                    /*TODO: use COAP_EVENT from core/iwasm/lib/app-libs/base/wasm_app.h instead of 6 */
                    if (packet->code == /*COAP_EVENT*/6)
                    handle_event(msg);
                }
#else
                /*TODO: use COAP_EVENT from core/iwasm/lib/app-libs/base/wasm_app.h instead of 6 */
                if (((coap_packet_t *) msg->payload)->code == /*COAP_EVENT*/6)
                    handle_event(msg);
                else
                    handle_response(msg);
#endif
            }
            memset(&recv_ctx, 0, sizeof(recv_ctx));
        }
        p++;
    }

    return 0;
}
