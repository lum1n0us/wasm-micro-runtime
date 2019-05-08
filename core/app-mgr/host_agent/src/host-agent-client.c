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
 * @file   host-agent-client.c
 *
 * @brief Handle message from or send message to host client.
 */

#include "host-agent-client.h"

#include "host-agent.h"
#include "host-agent-log.h"
#include "host-agent-utils.h"
#include "host-agent-callback.h"
#include "host-agent-aee.h"
#include "host-agent-imrt-link.h"
#include "coap_ext.h"

#include "attr-container.h"
#include "transaction.h"
#include "bh_common.h"

extern host_client_t *g_host_clients;
extern sync_ctx_t *g_transaction_ctx;

event_t *g_events = NULL;
const host_client_t *daemon_client = NULL;

static bool str_starts_with(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

static aee_trans_ctx_t*
create_aee_trans_ctx(const uint8_t *token, uint8_t token_len,
        const host_client_t *client)
{
    aee_trans_ctx_t *trans_ctx = (aee_trans_ctx_t *) malloc(
            sizeof(aee_trans_ctx_t));
    if (NULL == trans_ctx) {
        LOG_FATAL("create_aee_trans_ctx: malloc fail for aee_trans_ctx_t\n");
        return NULL;
    }

    if (4 == token_len) {
        bh_memcpy_s(trans_ctx->token, sizeof(trans_ctx->token), token,
                token_len);
    } else {
        LOG_ERROR("create_aee_trans_ctx: invalid token len: %d\n", token_len);
        free(trans_ctx);
        return NULL;
    }

    trans_ctx->client = client;
    return trans_ctx;
}

static void handle_normal_request(host_client_t *client,
        coap_packet_t *coap_packet)
{
    unsigned long global_out_id = bh_gen_id(g_transaction_ctx);
    aee_trans_ctx_t *trans_ctx = create_aee_trans_ctx(coap_packet->token,
            coap_packet->token_len, client);

    client->first_request_arrived = true;

    if (NULL == trans_ctx) {
        LOG_FATAL("handle_normal_request: create_aee_trans_ctx fail\n");
        return;
    }

    coap_set_token(coap_packet, (unsigned char *) &global_out_id,
            sizeof(global_out_id));

    if (!host_agent_aee_send_coap_msg(coap_packet)) {
        LOG_ERROR("handle_normal_request: host_agent_aee_send_coap_msg fail\n");
        free(trans_ctx);
    } else {
        // TODO: use the timeout of JAVA reqeust or use large enough timeout
        bh_wait_response_async(g_transaction_ctx, global_out_id,
                cb_foward_aee_response_to_client, trans_ctx, 10 * 60 * 1000,
                NULL);
    }
}

static bool check_url(const char *url)
{
    // TODO:
    return true;
}

void host_agent_client_free_counted_buf(counted_buf_t *counted_buf)
{
    counted_buf->cnt--;
    if (counted_buf->cnt <= 0) {
        free(counted_buf->buf);
        free(counted_buf);
    }
}

static bool contain_url(const char *url)
{
    for (event_t *cur = g_events; cur != NULL; cur = cur->next) {
        if (strcmp(cur->url, url) == 0) {
            return true;
        }
    }
    return false;
}

static bool send_response_to_client(const host_client_t *client, int mid,
        uint8_t code, const char *msg)
{
    bool ret;
    unsigned payload_len = 0;
    attr_container_t *payload = NULL;

    if (msg) {
        if ((payload = attr_container_create("")) == NULL
                || !attr_container_set_string(&payload, "error message", msg)) {
            ret = false;
            goto clean;
        } else {
            payload_len = attr_container_get_serialize_length(payload);
        }
    }

    ret = host_agent_client_send_request(client, mid, NULL, code,
            (uint8_t*) payload, payload_len);

    clean: attr_container_destroy(payload);
    return ret;
}

static void register_event(const char *url, host_client_t *client,
        coap_packet_t *msg)
{
    event_t *current = g_events, *node = NULL;

    LOG_VERBOSE("register_event adding url:(%s) sock:%d\n", url, client->sock);

    if (!check_url(url)) {
        LOG_ERROR("register_event: invaild url:(%s)\n", url);
        return;
    }

    while (current != NULL) {
        if ((strcmp(current->url, url) == 0) && (current->client == client)) {
            if (current->disabled) {
                LOG_INFO("register_event: enable url:(%s) sock:%d\n", url,
                        client->sock);
                current->disabled = false;

                // send buffered event
                if (current->event_buf_list != NULL) {
                    event_buf_t *pre_buf, *cur_buf, *next_buf;

                    // reverse list to send old event first
                    pre_buf = NULL;
                    cur_buf = current->event_buf_list;
                    next_buf = NULL;
                    while (cur_buf != NULL) {
                        next_buf = cur_buf->next;
                        cur_buf->next = pre_buf;
                        pre_buf = cur_buf;
                        cur_buf = next_buf;
                    }
                    current->event_buf_list = pre_buf;

                    LOG_INFO(
                            "forward buffered event to client: sock:%d size:%d\n",
                            client->sock, current->list_size);
                    cur_buf = current->event_buf_list;
                    while (cur_buf != NULL) {
                        LOG_INFO("buf len:%d\n", cur_buf->counted_buf->len);
                        host_agent_client_send_msg(client,
                                cur_buf->counted_buf->buf,
                                cur_buf->counted_buf->len);
                        next_buf = cur_buf->next;
                        host_agent_client_free_counted_buf(
                                cur_buf->counted_buf);
                        free(cur_buf);
                        cur_buf = next_buf;
                    }
                    current->event_buf_list = NULL;
                    current->list_size = 0;
                }
            } else {
                LOG_WARNING(
                        "register_event: cannot add duplicated url:(%s) sock:%d\n",
                        url, client->sock);
            }
            return;
        }
        current = current->next;
    }

    if (NULL
            == (node = (event_t *) malloc(
                    offsetof(event_t, url) + strlen(url) + 1))) {
        LOG_FATAL("register_event: malloc fail\n");
        return;
    }

    if (!contain_url(url)) {
        handle_normal_request(client, msg);
    } else {
        send_response_to_client(client, *(int*) msg->token, CONTENT_2_05, NULL);
    }

    memset(node, 0, sizeof(event_t));
    bh_strcpy_s(node->url, strlen(url) + 1, url);
    node->client = client;
    node->next = g_events;
    g_events = node;

    LOG_VERBOSE("register_event added url:(%s) sock:%d\n", url, client->sock);
}

static void free_event(event_t *e)
{
    event_buf_t *cur, *next;
    cur = e->event_buf_list;
    while (cur != NULL) {
        next = cur->next;
        host_agent_client_free_counted_buf(cur->counted_buf);
        free(cur);
        cur = next;
    }
    free(e);
}

static void unregister_event(const char *url, host_client_t *client,
        coap_packet_t *msg)
{
    bool found = false;
    event_t *current = g_events, *pre = NULL;

    LOG_VERBOSE("unregister_event deleting url:(%s) sock:%d\n", url,
            client->sock);

    if (!check_url(url)) {
        LOG_ERROR("unregister_event: invaild url:(%s)\n", url);
        return;
    }

    while (current != NULL) {
        if ((strcmp(current->url, url) == 0) && (current->client == client)) {
            if (pre)
                pre->next = current->next;
            else
                g_events = current->next;

            found = true;
            if (!contain_url(current->url)) {
                handle_normal_request(client, msg);
            } else {
                send_response_to_client(client, *(int*) msg->token,
                        CONTENT_2_05, NULL);
            }
            free_event(current);
            break;
        }
        pre = current;
        current = current->next;
    }

    if (!found)
        LOG_VERBOSE("unregister_event not found url:(%s) sock:%d\n", url,
                client->sock);
    else
        LOG_VERBOSE("unregister_event deleted url:(%s) sock:%d\n", url,
                client->sock);
}

static void handle_event_request(host_client_t *client, coap_packet_t *msg,
        const char *event_url)
{
    client->first_request_arrived = true;

    if (msg->code == COAP_PUT) { /* register */
        register_event(event_url, client, msg);
    } else if (msg->code == COAP_DELETE) { /* unregister */
        unregister_event(event_url, client, msg);
    } else {
        /* invalid request */
    }
}

static void handle_package_name_request(host_client_t *client,
        const char *package_name)
{
    LOG_VERBOSE("handle_package_name_request: sock: %d name: (%s)\n",
            client->sock, package_name);

    if (client->first_request_arrived) {
        // Package name request is sent by HostClient immediately after connection is established,
        // so this way prevents app developer from sending such a request with malicious name.
        LOG_WARNING(
                "handle_package_name_request: only the first request is allowed to set package name\n");
        return;
    }

    client->first_request_arrived = true;

    if (package_name[0] == '\0') {
        // empty package_name is sent when no AEE.permission.wake_on_event
        // place here to ensure first_request_arrived is set
        return;
    }

    for (host_client_t *c = g_host_clients; c != NULL; c = c->next) {
        if (c->package_name != NULL
                && strcmp(c->package_name, package_name) == 0) {
            // app reconnect
            LOG_INFO("handle_package_name_request: app reconnect\n");
            if (c->sock >= 0) {
                LOG_WARNING(
                        "handle_package_name_request: app reconnect when previous connection is not closed\n");
                close(c->sock);
                c->recv_ctx.phase = Phase_Non_Start;
            }

            c->sock = client->sock;
            host_agent_client_destroy_client(client);
            c->wake_timestamp = 0;
            return;
        }
    }

    client->package_name = strdup(package_name);
    if (client->package_name == NULL) {
        LOG_FATAL("handle_package_name_request: strdup fail\n");
        return;
    }

    if (strcmp(package_name, DAEMON_PACKAGE) == 0) {
        daemon_client = client;
    }
}

void host_agent_client_handle_msg(host_client_t *client,
        const imrt_link_message_t *link_message)
{
    erbium_status_code = NO_ERROR;
    coap_packet_t message[1];

    LOG_INFO("host_agent_client_handle_msg\n");

    /* so far, no other message type than coap tcp */
    if (link_message->message_type != COAP_TCP_RAW) {
        LOG_ERROR("host_agent_client_handle_msg: unsupported message type %d\n",
                link_message->message_type);
        return;
    }

    erbium_status_code = coap_parse_message_tcp(message, link_message->payload,
            link_message->payload_size);

    if (erbium_status_code != NO_ERROR) {
        LOG_ERROR(
                "host_agent_client_handle_msg: coap_parse_message_tcp fail with error:%d\n",
                erbium_status_code);
        return;
    }

    /* handle requests */
    if (message->code >= COAP_GET && message->code <= COAP_DELETE) {
        char url_allocated[256] = { 0 };
        const char *url = NULL;
        int url_len = coap_get_header_uri_path(message, &url);

        if (url_len == 0 || url_len >= sizeof(url_allocated)) {
            LOG_ERROR(
                    "host_agent_client_handle_msg: url too short or too long. len:%d url:[%s]\n",
                    url_len, url);
            return;
        }

        bh_memcpy_s(url_allocated, sizeof(url_allocated) - 1, url, url_len);

        LOG_VERBOSE("host_agent_client_handle_msg: url_allocated:(%s)\n",
                url_allocated);

        if (str_starts_with("/event/", url_allocated)) {
            char *event_url = url_allocated + strlen("/event/");
            if (event_url[0] == '\0') {
                /* empty event string("/event/") should not be registered */
                LOG_WARNING(
                        "host_agent_client_handle_msg: empty event string:(%s)\n",
                        url_allocated);
                return;
            }
            handle_event_request(client, message, event_url);
        } else if (str_starts_with("/package_name/", url_allocated)) {
            char *package_name = url_allocated + strlen("/package_name/");
            if (message->code != COAP_PUT) {
                LOG_WARNING("host_agent_client_handle_msg: "
                        "invalid action for package name request: %hd\n",
                        message->code);
                return;
            }
            handle_package_name_request(client, package_name);
        } else if (strcmp("/close", url_allocated) == 0) {
            close(client->sock);
            host_agent_client_destroy_client(client);
        } else {
            handle_normal_request(client, message);
            LOG_WARNING("host_agent_client_handle_msg: unknown url:(%s)\n",
                    url_allocated);
        }
    }
}

bool host_agent_client_send_msg(const host_client_t *client, const void *buf,
        size_t len)
{
    int cnt = 0;
    ssize_t ret;

    LOG_INFO("host_agent_client_send_msg begin sock:%d buf:%p len:%d\n",
            client->sock, buf, len);

    if (client->sock < 0 || buf == NULL || len <= 0) {
        return false;
    }

    resend: ret = send(client->sock, buf, len, 0);

    if (ret == -1) {
        LOG_ERROR("host_agent_client_send_msg fail with error: %d\n", errno);

        // repeat sending if the outbuffer is full
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (++cnt > 10) {
                return false;
            }
            sleep(1);
            goto resend;
        }
    }

    LOG_INFO("host_agent_client_send_msg done sent:%d\n", ret);

    return (ret == len);
}

bool host_agent_client_send_coap_msg(const host_client_t *client,
        const coap_packet_t *msg)
{
    int len;
    char *buf;
    bool res;

    if (client == NULL || msg == NULL)
        return false;

    len = serialize_coap_packet_to_imrt_link(msg, &buf);
    if (len < 0) {
        LOG_ERROR(
                "host_agent_client_send_coap_msg: serialize_coap_packet_to_imrt_link fail %d\n",
                len);
        return false;
    }

    res = host_agent_client_send_msg(client, buf, len);

    free(buf);

    return res;
}

bool host_agent_client_send_request(const host_client_t *client, int mid,
        const char *url, uint8_t code, const uint8_t *payload,
        uint32_t payload_len)
{
    coap_packet_t packet;

    coap_init_message(&packet, COAP_TYPE_NON, code, mid);
    coap_set_token(&packet, (uint8_t*) &mid, sizeof(mid));
    coap_set_header_content_format(&packet, 0);

    if (url)
        coap_set_header_uri_path(&packet, url);

    if (payload && payload_len > 0)
        coap_set_payload(&packet, payload, payload_len);

    return host_agent_client_send_coap_msg(client, &packet);
}

void host_agent_client_wake_client(host_client_t *client)
{
    LOG_VERBOSE("host_agent_client_wake_client: sock: %d\n", client->sock);

    if (daemon_client != NULL && client->package_name != NULL) {
        attr_container_t *payload;

        if ((payload = attr_container_create(NULL)) == NULL) {
            LOG_FATAL(
                    "host_agent_client_wake_client: creating attr container fail\n");
            return;
        }
        if (!attr_container_set_string(&payload, "package_name",
                client->package_name)) {
            LOG_FATAL("host_agent_client_wake_client: setting attr fail\n");
            attr_container_destroy(payload);
            return;
        }

        if (!host_agent_client_send_request(daemon_client, 0, "/wakeup_app",
                COAP_PUT, (uint8_t*) payload,
                attr_container_get_serialize_length(payload))) {
            LOG_ERROR("host_agent_client_wake_client: sending msg fail\n");
            attr_container_destroy(payload);
            return;
        }

        attr_container_destroy(payload);

        client->sock = CLIENT_WAKED;
        client->wake_timestamp = time(NULL);
    } else {
        LOG_ERROR("should not reach here, maybe daemon is disconnected\n");
    }
}

int host_agent_client_init(const char *p)
{
    uint16_t port = atoi(p);
    return listen_tcp_port(port);
}

int host_agent_client_add_client(int sock)
{
    host_client_t * client = (host_client_t *) malloc(sizeof(host_client_t));

    if (client == NULL) {
        LOG_ERROR("alloc new client fail\n");
        return -1;
    }

    memset(client, 0, sizeof(host_client_t));
    client->sock = sock;
    client->next = g_host_clients;
    g_host_clients = client;

    return 0;
}

static void destroy_client_transactions(const host_client_t *client)
{
    /* free transactions belong to this client */
    if (g_transaction_ctx != NULL) {
        sync_node_t *current = (sync_node_t *) g_transaction_ctx->list, *pre =
        NULL, *node;
        while (current != NULL) {
            aee_trans_ctx_t *trans_ctx;
            if (current->sync_type == 2 && current->cb_data.work_thread == NULL
                    && (trans_ctx =
                            (aee_trans_ctx_t *) current->cb_data.context_data)
                            != NULL && trans_ctx->client == client) {
                LOG_VERBOSE("free a transaction sock:%d token:%d\n",
                        trans_ctx->client->sock,
                        ntohl(*((int * )trans_ctx->token)));
                if (pre)
                    pre->next = current->next;
                else
                    g_transaction_ctx->list = current->next;
                node = current;
                current = current->next;
                free(trans_ctx);
                free(node);
            } else {
                pre = current;
                current = current->next;
            }
        }
    }
}

static void disable_client(host_client_t *client)
{
    LOG_VERBOSE("disable_client: sock: %d\n", client->sock);

    client->sock = CLIENT_DISCONNECTED;
    client->recv_ctx.phase = Phase_Non_Start;

    /* disable events belongs to this client */
    for (event_t *cur = g_events; cur != NULL; cur = cur->next) {
        if (cur->client == client) {
            LOG_VERBOSE("disable an event sock:%d url:(%s)\n",
                    cur->client->sock, cur->url);
            cur->disabled = true;
        }
    }

    destroy_client_transactions(client);
}

static void unregister_event_to_aee(char *url)
{
    char *buf = (char *) malloc(strlen("/event/") + strlen(url) + 1);
    if (buf == NULL) {
        LOG_FATAL("unregister_event_to_aee: malloc fail\n");
    } else {
        bh_strcpy_s(buf, strlen("/event/") + 1, "/event/");
        bh_strcat_s(buf, strlen("/event/") + strlen(url) + 1, url);
        host_agent_aee_send_request(bh_gen_id(g_transaction_ctx), buf,
                COAP_DELETE, NULL, 0);
        free(buf);
    }
}

void host_agent_client_destroy_client(host_client_t *client)
{
    LOG_VERBOSE("host_agent_client_destroy_client: sock: %d\n", client->sock);

    if (client->recv_ctx.message.payload)
        free(client->recv_ctx.message.payload);

    /* free events belongs to this client */
    if (g_events != NULL) {
        event_t *current = g_events, *pre = NULL, *node;

        while (current != NULL) {
            if (current->client == client) {
                if (pre)
                    pre->next = current->next;
                else
                    g_events = current->next;

                node = current;
                current = current->next;
                if (!contain_url(node->url)) {
                    unregister_event_to_aee(node->url);
                }
                LOG_VERBOSE("free an event sock:%d url:(%s)\n",
                        node->client->sock, node->url);
                free_event(node);
            } else {
                pre = current;
                current = current->next;
            }
        }
    }

    destroy_client_transactions(client);

    free(client->package_name);
    {
        host_client_t *current = g_host_clients, *pre = NULL;
        while (current != NULL) {
            if (current == client) {
                if (pre)
                    pre->next = current->next;
                else
                    g_host_clients = current->next;

                free(current);
                break;
            }
            pre = current;
            current = current->next;
        }
    }
}

static bool client_contain_event(const host_client_t *client)
{
    for (event_t *cur = g_events; cur != NULL; cur = cur->next) {
        if (cur->client == client) {
            return true;
        }
    }
    return false;
}

void host_agent_client_clear_client(host_client_t *client)
{
    if (client == daemon_client) {
        LOG_ERROR("daemon should not disconnect\n");
        daemon_client = NULL;
    }

    close(client->sock);
    if (daemon_client != NULL && client->package_name != NULL
            && client_contain_event(client)) {
        disable_client(client);
    } else {
        host_agent_client_destroy_client(client);
    }
}

void host_agent_client_destroy_clients()
{
    LOG_VERBOSE("destroy clients\n");

    /* free Host Client */
    LOG_VERBOSE("free Host Clients\n");
    if (g_host_clients != NULL) {
        host_client_t *current = g_host_clients;
        while (current != NULL) {
            host_client_t *node = current;
            current = current->next;
            free(node->package_name);
            free(node);
        }
    }

    /* free transaction nodes */
    LOG_VERBOSE("free transactions\n");
    if (g_transaction_ctx != NULL) {
        sync_node_t *current = (sync_node_t *) g_transaction_ctx->list;
        while (current != NULL) {
            sync_node_t *node = current;
            aee_trans_ctx_t *trans_ctx;
            if (current->sync_type
                    == 2&& current->cb_data.work_thread == NULL
                    && (trans_ctx = (aee_trans_ctx_t *) current->cb_data.context_data) != NULL) {
                free(trans_ctx);
            }
            current = current->next;
            free(node);
        }
        delete_sync_ctx(g_transaction_ctx);
    }

    /* free events */
    LOG_VERBOSE("free events\n");
    if (g_events != NULL) {
        event_t *current = g_events;
        while (current != NULL) {
            event_t *node = current;
            current = current->next;
            g_events = current;
            if (!contain_url(node->url)) {
                unregister_event_to_aee(node->url);
            }
            free_event(node);
        }
    }
}

void host_agent_client_destroy()
{
    host_agent_client_destroy_clients();
}
