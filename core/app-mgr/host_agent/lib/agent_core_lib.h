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

#ifndef APPS_IAGENT_CORE_LIB_AGENT_CORE_LIB_H_
#define APPS_IAGENT_CORE_LIB_AGENT_CORE_LIB_H_

#include "iagent_bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define S_PENDING_DELETE  1

#define COUNT_OF(x)  (sizeof(x)/sizeof(x[0]))

//message_queue
#define MSG_NOW time(NULL)

/************************************************************************/
/*                                                                      */
/*                           transaction.c                              */
/*                                                                      */
/************************************************************************/

#ifndef sync_t
//typedef int sync_t;
#endif

enum {
    T_Empty,
    T_Raw,
    T_Coap_Raw,
    T_Coap_Parsed,
    T_iLink_Parsed,
    T_Broker_Message_Handle,

    T_Trans_User_Fmt = 100
};

typedef int (*bh_async_callback)(void * ctx, void * data, int len,
        unsigned char format);

typedef struct _callback {
    void * data;
    int len;
    unsigned char format;
    void * transaction;
} callback_t;
void execute_callback_node(callback_t * cb);

typedef void (*bh_post_callback)(callback_t * callback);

typedef struct sync_ctx {
    ptr_sync_t ctx_lock;
    unsigned int cnt;
    void * list;
    time_t last_check;
    uint32_t new_id;
} sync_ctx_t;

//transaction
//#define CTX_BUFFER_CECK 1
#ifdef CTX_BUFFER_CECK
void * trans_malloc_ctx(int len);
void trans_free_ctx(void *);
#else
#define trans_malloc_ctx malloc
#define trans_free_ctx free
#endif

sync_ctx_t * create_sync_ctx();
void delete_sync_ctx(sync_ctx_t* ctx);
int bh_wait_response(sync_ctx_t * sync_ctx, uint32_t id, void ** response,
        uint32_t timeout);
void bh_feed_response(sync_ctx_t * sync_ctx, uint32_t id, void * response,
        uint32_t len, uint8_t format);
uint32_t bh_wait_response_async(sync_ctx_t * sync_ctx, uint32_t id, /*bh_async_callback*/
void* cb, void* ctx_data, uint32_t timeout, void * worker_thread);

uint32_t bh_handle_expired_trans(sync_ctx_t* sync_ctx);
unsigned long bh_gen_id(sync_ctx_t * ctx);
void execute_callback_node(callback_t * cb);
sync_ctx_t* get_outgoing_requests_ctx();

#ifdef __cplusplus
}
#endif

#endif /* APPS_IAGENT_CORE_LIB_AGENT_CORE_LIB_H_ */
