#include <time.h>
#include <sys/time.h>
#include "transaction_manager.h"
#include "er-coap.h"
#include "bh_common.h"
#include "attr-container.h"
#include "imrt_link_message.h"

event_list *g_event_listener = NULL;
transaction *g_transaction_list_header = NULL;
extern int sockfd;
extern pthread_mutex_t mutex;

bool aee_request_send(aee_request_t *req, aee_response_handler_t handler,
        void *usr_ctx, int timeout)
{
    int ret = 0;
    req->mid = generate_mid();
    printf("send req mid:%d", req->mid);
    struct timeval time_stamp;
    gettimeofday(&time_stamp, NULL);
    transaction *trans = transaction_create(req->mid, req, handler, usr_ctx,
            timeout, time_stamp.tv_sec * 1000 + time_stamp.tv_usec / 1000);
    char *buf = NULL;
    int length;
    transaction_encode(trans, &buf, &length);
    char *imrt_link_msg_buf = NULL;
    char *imrt_link_message = NULL;
    int imrt_link_message_length = 0;
    imrt_link_message_encode(buf, length, &imrt_link_message,
            &imrt_link_message_length);
    ret = send_to_host_agent(imrt_link_message, imrt_link_message_length);

    free(buf);
    free(imrt_link_message);

    return ret == 0 ? true : false;
}
#if 1
int transaction_encode(transaction *req, char **buf, int *length)
{
    coap_packet_t message[1];
    int len = 0;
    coap_init_message(message, COAP_TYPE_CON, req->request->action,
            coap_get_mid());
    coap_set_header_uri_path(message, req->request->url);
    coap_set_header_content_format(message, 0);
    //coap_set_header_uri_query(message, "/ams/v1/c/d");
    int buf_size = offsetof(aee_request_t, payload)
            + req->request->payload_size;
    int offset = offsetof(aee_request_t, payload);
    char *req_buf = malloc(buf_size);
    memcpy(req_buf, req->request, offset);               //copy structure header
    memcpy(req_buf + offset, req->request->payload, req->request->payload_size); //copy payload
    coap_set_payload(message, req->request->payload,
            req->request->payload_size);
    coap_set_token(message, &req->request->mid, sizeof(req->request->mid));
    len = coap_serialize_message_tcp(message, buf);
    free(req_buf);
    *length = len;
    return len;
}
#endif

int transaction_add(transaction *trans)
{
    trans->next = g_transaction_list_header; // add to header
    g_transaction_list_header = trans;
    return 0;
}

transaction *transaction_create(int mid, aee_request_t *req,
        aee_response_handler_t handler, void *usr_ctx, int timeout,
        unsigned long long time_stamp_msec)
{
    transaction *node = malloc(sizeof(transaction));
    memset(node, 0, sizeof(transaction));
    node->mid = mid;
    node->request = req;
    node->handler = handler;
    node->usr_ctx = usr_ctx;
    node->timeout = timeout;
    node->startTime = time_stamp_msec;
    transaction_add(node);
    return node;
}

int event_add(char *url, aee_event_listener_t handler)
{
    event_list *node = malloc(sizeof(event_list));
    memset(node, 0, sizeof(event_list));
    node->url = strdup(url);
    node->evt = handler;

    node->next = g_event_listener; // add to header
    g_event_listener = node;
    return 0;
}

event_list *event_find_by_url(char *url)
{
    event_list *n = g_event_listener;
    if (g_event_listener == NULL) {
        return 0;
    }

    while (n) {
        if (0 == strcmp(g_event_listener->url, url)) {
            return n;
        }
        n = n->next;
    }
}

int event_remove_by_url(char *url)
{
    event_list *node;
    event_list *n = g_event_listener;
    if (g_event_listener == NULL) {
        return -1;
    }
    if (0 == strcmp(g_event_listener->url, url)) {
        g_event_listener = g_event_listener->next;
        return 0;
    }

    while (n) {
        if (0 == strcmp(n->next->url, url)) {
            node = n->next;
            n->next = node->next;
            if (node->url != NULL) {
                free(node->url);
            }
            free(node);
            return 0;
        }
        n = n->next;
    }
    return -1;
}
int transaction_remove(transaction *trans)
{
    transaction *node;
    transaction *n = g_transaction_list_header;
    if (g_transaction_list_header == trans) {
        //node = g_transaction_list_header;
        g_transaction_list_header = g_transaction_list_header->next;
        return 0;
    }

    while (n) {
        if (n->next == trans) {
            node = n->next;
            n->next = node->next;
            break;
        }
        n = n->next;
    }
}
int transaction_remove_by_mid(int mid)
{
    transaction *trans = transaction_find_by_mid(mid);
    if (trans != NULL) {
        transaction_remove(trans);
    }
}

char recv_buf[1024 * 1024];
#define DEFAULT_BUFLEN 4096

void *host_api_recv_thread()
{
    int ret = 0;
    int packet_len = 0;
    int recvbuflen = DEFAULT_BUFLEN;
    char recved[8192];

    fd_set stReadFDS = { 0 };

    FD_ZERO(&stReadFDS);

    struct timeval current_time_stamp;
    struct timeval tv;

    for (;;) {
        gettimeofday(&current_time_stamp, NULL);
        handle_expired_transactions(
                current_time_stamp.tv_sec * 1000
                        + current_time_stamp.tv_usec / 1000);
        //pthread_mutex_lock(&mutex);
        FD_SET(sockfd, &stReadFDS);
        tv.tv_usec = 0;
        tv.tv_sec = 1;
        ret = select(sockfd + 1, &stReadFDS, NULL, NULL, &tv);

        if (ret > 0) {
            if (FD_ISSET(sockfd, &stReadFDS)) {
                packet_len = recv(sockfd, recved, recvbuflen, 0);
                if (packet_len >= 0) {
                    aee_host_msg_callback(recved, packet_len);
                }
            }
        } else if (ret == SO_ERROR) {
            printf("socket error\n");
        } else if (ret == 0) {
            //printf("select timeout\n");
        }
        //pthread_mutex_unlock(&mutex);
    }
}

void handle_expired_transactions(unsigned long long current_time_stamp_msec)
{

    transaction *n = g_transaction_list_header;
    while (n) {
        printf("current time stamp :%lld\n", current_time_stamp_msec);
        printf("request start time stamp :%lld\n", n->startTime);
        printf("time elapsed:%lld\n", current_time_stamp_msec - n->startTime);
        if ((current_time_stamp_msec - n->startTime) > n->timeout) {
            printf("removed expired requests\n");
            n->handler(n->usr_ctx, NULL);
            transaction_remove(n);
        }
        n = n->next;
    }
}

void handle_response(bh_queue_msg_t *message)
{
    coap_packet_t *packet = (coap_packet_t *) message->payload;
    attr_container_t *attr_cont = (attr_container_t *) packet->payload;
    const char *url = NULL;
    int url_len = 0, mid;
    transaction *trans;

    if (message == NULL || message->payload == NULL)
        return;
    printf("handle an response message from host agent\n");

    if (message->message_type != COAP_PARSED) {
        printf("unkown message type: \n");
        return;
    }

    bh_memcpy_s(&mid, sizeof(unsigned int), packet->token,
            sizeof(unsigned int));

    printf("response mid:%d\n", mid);
    trans = transaction_find_by_mid(mid);
    if (NULL != trans) {
        aee_response_handler_t handler;
        aee_response_t response;
        response.fmt = PAYLOAD_FORMAT_ATTRIBUTE_OBJECT; //todo
        response.mid = mid;
        response.status = packet->code;
        response.payload = (char *) attr_cont;
        handler = trans->handler;
        handler(trans->usr_ctx, &response);
        transaction_remove(trans); //remove transaction
    }
}

void handle_event(bh_queue_msg_t *message)
{
    coap_packet_t *packet = (coap_packet_t *) message->payload;
    attr_container_t *attr_cont = (attr_container_t *) packet->payload;
    const char *url = NULL;
    int url_len = 0, mid;

    if (message == NULL || message->payload == NULL)
        return;
    printf("handle an event message from host agent\n");

    if (message->message_type != COAP_PARSED) {
        printf("unkown message type: \n");
        return;
    }

    bh_memcpy_s(&mid, sizeof(unsigned int), packet->token,
            sizeof(unsigned int));

    url_len = coap_get_header_uri_path(packet, &url);
    char *url_end_with_zero = NULL;
    url_end_with_zero = malloc(url_len + 1);
    memcpy(url_end_with_zero, url, url_len);
    url_end_with_zero[url_len] = 0;
    event_list *evt = event_find_by_url((char *) url_end_with_zero);
    if (evt != NULL) {
        evt->evt(url_end_with_zero, packet->payload, /*PAYLOAD_FMT*/
        packet->content_format);
    }
    free(url_end_with_zero);
}

transaction *transaction_find_by_mid(int mid)
{
    transaction *n = g_transaction_list_header;
    while (n) {
        if (n->mid == mid) {
            return n;
        }
        n = n->next;
    }
}
