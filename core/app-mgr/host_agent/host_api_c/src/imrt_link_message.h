#ifndef IMRT_LINK_MESSAGE_H
#define IMRT_LINK_MESSAGE_H
#include "basic_types.h"
/* Receiving Phase */
typedef enum recv_phase_t {
    Phase_Non_Start, Phase_Leading, Phase_Type, Phase_Size, Phase_Payload
} recv_phase_t;

/* Queue message type */
typedef enum QUEUE_MSG_TYPE {
    COAP_TCP_RAW = 0,
    COAP_UDP_RAW = 1,
    COAP_PARSED = 2,
    APPLET_REQUEST = 3,
    APPLET_RESPONSE = 4,
    TIMER_EVENT = 5,
    SENSOR_EVENT = 6,
    GPIO_INTERRUPT_EVENT = 7,
    BLE_EVENT = 8,
    JDWP_REQUEST = 9
} QUEUE_MSG_TYPE;

/* Link message, or message between host and app manager */
typedef struct bh_link_msg_t {
    /* 2 bytes leading */
    uint16_t leading_bytes;
    /* message type, must be COAP_TCP or COAP_UDP */
    uint16_t message_type;
    /* size of payload */
    uint32_t payload_size;
    char *payload;
} bh_link_msg_t;

typedef struct bh_queue_msg_t {
    /* message type, should be one of QUEUE_MSG_TYPE */
    int32_t message_type;
    /* payload size */
    uint32_t payload_size;
    char *payload;
} bh_queue_msg_t;

/* Receive Context */
typedef struct recv_context_t {
    recv_phase_t phase;
    bh_link_msg_t message;
    int size_in_phase;
} recv_context_t;

extern int imrt_link_message_encode(char *input, int in_len, char **output,
        int *out_len);
extern int aee_host_msg_callback(void *msg, uint16_t msg_len);

#endif
