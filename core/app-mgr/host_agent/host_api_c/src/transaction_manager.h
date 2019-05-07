#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "host_api.h"
typedef struct transaction_t {
    int mid;
    aee_request_t *request;
    aee_response_handler_t handler;
    void *usr_ctx; /* request's label */
    int timeout;
    unsigned long long startTime;
    struct transaction_t *next;
} transaction;

typedef struct event_list_t {
    char *url;
    aee_event_listener_t evt;
    struct event_list_t *next;
} event_list;
extern event_list *g_event_listener;
extern transaction *g_transaction_list_header;

extern transaction *transaction_create(int mid, aee_request_t *req,
        aee_response_handler_t handler, void *usr_ctx, int timeout,
        unsigned long long start_time);
extern int transaction_encode(transaction *req, char **buf, int *length);
extern void *host_api_recv_thread();
extern bool aee_request_send(aee_request_t *req, aee_response_handler_t handler,
        void *usr_ctx, int timeout);
extern int sockfd;
//extern void handle_iMRT_link_message(bh_queue_msg_t * msg);
extern transaction *transaction_find_by_mid(int mid);
extern void handle_expired_transactions(unsigned long long current_time_stamp);
#endif
