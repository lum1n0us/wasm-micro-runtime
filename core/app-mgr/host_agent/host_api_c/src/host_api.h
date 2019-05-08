#ifndef HOST_API_H
#define HOST_API_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAYLOAD_FORMAT_ATTRIBUTE_OBJECT 0
#define PAYLOAD_FORMAT_JSON 1
#define PAYLOAD_FMT PAYLOAD_FORMAT_ATTRIBUTE_OBJECT

typedef struct aee_request_t {
    int mid;
    char *url;
    int action;
    int payload_fmt;
    int payload_size;
    char *payload;
} aee_request_t;

typedef struct aee_response_t {
    int mid;
    int status;
    int fmt;
    int payload_size;
    char *payload;
} aee_response_t;

typedef struct aee_applet_info_t {
    char name[256];
} aee_applet_info_t;

typedef struct aee_applet_list_t {
    int cnt;
    aee_applet_info_t *applets;
} aee_applet_list_t;

typedef void (*aee_event_listener_t)(const char *url, void *event, int fmt);

typedef void (*aee_response_handler_t)(void *usr_ctx, aee_response_t *response);

bool hostclient_initialize(const char *ip, unsigned short port);
void hostclient_shutdown();
bool hostclient_register_event(const char *url, aee_event_listener_t listener);
bool hostclient_unregister_event(const char *url);
void aee_request_init(aee_request_t *req, char *url, int action);
void aee_request_set_payload(aee_request_t *req, void *payload, int size,
        int fmt);
bool aee_request_send(aee_request_t *request, aee_response_handler_t handler,
        void *usr_ctx, int timeout_ms);
void *aee_request_get_payload(aee_request_t *request, int *size, int *fmt);
bool aee_applet_install(char *buf, char *module_type, int length,
        char *applet_name, int time_out);
bool aee_applet_uninstall(char *applet_name, char *module_type, int time_out);
bool aee_applet_list(int time_out, aee_applet_list_t *applet_list);
void aee_applet_list_init(aee_applet_list_t *applet_list);
void aee_applet_list_clean(aee_applet_list_t *applet_list);
const char *aee_applet_last_error();
int aee_response_get_status(aee_response_t *response);
void *aee_response_get_payload(aee_response_t *response, int *size, int *fmt);
#endif
