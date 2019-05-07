/*
 * transport.h
 *
 *  Created on: Apr 20, 2019
 *      Author: shaka
 */

#ifndef DEPS_APP_MGR_HOST_TOOL_SRC_TRANSPORT_H_
#define DEPS_APP_MGR_HOST_TOOL_SRC_TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned short message_type;
    unsigned long payload_size;
    char *payload;
} imrt_link_message_t;

typedef enum {
    Phase_Non_Start, Phase_Leading, Phase_Type, Phase_Size, Phase_Payload
} recv_phase_t;

typedef struct {
    recv_phase_t phase;
    int size_in_phase;
    imrt_link_message_t message;
} imrt_link_recv_context_t;

#ifdef __cplusplus
} /* end of extern "C" */
#endif

bool host_tool_send_data(int fd, const char *buf, unsigned int len);

int on_imrt_link_byte_arrive(unsigned char ch, imrt_link_recv_context_t *ctx);

bool tcp_init(const char *address, uint16_t port, int *fd);
bool uart_init(const char *conn_str, int *fd);

bool udp_send(const char *address, int port, const char *buf, int len);

#endif /* DEPS_APP_MGR_HOST_TOOL_SRC_TRANSPORT_H_ */
