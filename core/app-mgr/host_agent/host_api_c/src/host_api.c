#include <stdio.h>
#include "host_api.h"
#include "er-coap.h"
#include "attr-container.h"
#include "imrt_link_message.h"
#include "transaction_manager.h"

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <error.h>

#include "basic_types.h"

int flag_install_applet_success_flag = 0;
int flag_uninstall_applet_success_flag = 0;
int g_mid = 0;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern void register_applet_exception_event();
extern void unregister_applet_exception_event();
int sockfd;
struct hostent *he;
struct sockaddr_in sockaddr_bind;

typedef struct query_applet_usr_ctx_t {
    void *condition;
    aee_applet_list_t applets;
} query_applet_usr_ctx;

int generate_mid()
{
    return g_mid++;
}

void f_aee_install_applet_response_handler(void *usr_ctx,
        aee_response_t *response)
{

    if (CREATED_2_01 == aee_response_get_status(response)) {
        flag_install_applet_success_flag = 1;
    } else {
        flag_install_applet_success_flag = 0;
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(usr_ctx);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    printf("f_aee_install_applet_response_handler \n");
}

void f_aee_uninstall_applet_response_handler(void *usr_ctx,
        aee_response_t *response)
{
    if (DELETED_2_02 == aee_response_get_status(response)) {
        flag_uninstall_applet_success_flag = 1;
    } else {
        flag_uninstall_applet_success_flag = 0;
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(usr_ctx);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    printf("f_aee_uninstall_applet_response_handler \n");
}

void f_aee_query_applet_response_handler(void *usr_ctx,
        aee_response_t *response)
{
    int cnt = 0;
    int i = 0;
    int size = 0;
    query_applet_usr_ctx *context = (query_applet_usr_ctx *) usr_ctx;

    attr_container_t *attr_obj = (attr_container_t *) response->payload;

    attr_container_dump(attr_obj);
    cnt = attr_container_get_as_int(attr_obj, "num");
    context->applets.cnt = cnt;
    if (cnt != 0) {
        size = cnt * sizeof(aee_applet_info_t);
        context->applets.applets = malloc(size);
        memset(context->applets.applets, 0, size);
        for (i = 0; i < cnt; i++) {
            char key[256] = { 0 };
            char post_index[128];
            sprintf(post_index, "%d", i + 1);
            strcat(key, "applet");
            strcat(key, post_index);
            char *name = attr_container_get_as_string(attr_obj, key);
            strcpy(context->applets.applets[i].name, name);
            //applet_list->applets->name
        }
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_signal(context->condition);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    printf("f_aee_query_applet_response_handler \n");
}

bool hostclient_initialize(const char *ip, unsigned short port)
{
    bool ret = false;

    // Get the host info
    if ((he = gethostbyname(ip)) == NULL) {
        perror("gethostbyname()");
        exit(1);
    } else
        printf("Client-The remote host is: %s\n", ip);

    // Create a SOCKET for connecting to server
    pthread_mutex_lock(&mutex);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create sockaet failed\n");
        return false;
    }
    pthread_mutex_unlock(&mutex);
    sockaddr_bind.sin_family = AF_INET;
    sockaddr_bind.sin_port = htons(port);
    //sockaddr_bind.sin_addr = *((struct in_addr *)he->h_addr);
    //memset(&(sockaddr_bind.sin_zero), '\0', 8);

    // Connect to server.
    pthread_mutex_lock(&mutex);
    if (connect(sockfd, (struct sockaddr *) &sockaddr_bind,
            sizeof(struct sockaddr)) == -1) {
        perror("connect()");
        ret = false;
    } else
        printf("Client-The connect() is OK...\n");

    pthread_mutex_unlock(&mutex);
    ret = true;

    if (!ret) {
        printf("host agent connect failed\n");
        return false;
    }

    pthread_t ntid;
    if (pthread_create(&ntid, NULL, host_api_recv_thread, NULL)) {
        printf("host agent create thread failed\n");
        return false;
    }

    return true;
}

void hostclient_shutdown()
{
    // cleanup
    pthread_mutex_lock(&mutex);
    close(sockfd);
    pthread_mutex_unlock(&mutex);
}

bool hostclient_register_event(const char *url, aee_event_listener_t listener)
{
    int ret = 0;
    event_add(url, listener);

    char *url_total[2048] = { 0 };
    if (url[0] == '/') {
        strcat(url_total, "/event");
    } else {
        strcat(url_total, "/event/");
    }
    strcat(url_total, url);

    char *buf = NULL;
    int len = 0;
    coap_packet_t message[1];
    int mid = generate_mid();
    coap_init_message(message, COAP_TYPE_CON, COAP_PUT, 0);
    coap_set_header_uri_path(message, url_total);
    coap_set_token(message, &mid, sizeof(mid));
    coap_set_header_content_format(message, 0);
    len = coap_serialize_message_tcp(message, &buf);

    char *imrt_link_message = NULL;
    int imrt_link_message_length = 0;
    imrt_link_message_encode(buf, len, &imrt_link_message,
            &imrt_link_message_length);
    ret = send_to_host_agent(imrt_link_message, imrt_link_message_length);

    free(buf);
    free(imrt_link_message);
}
bool hostclient_unregister_event(const char *url)
{
    int ret = 0;
    event_remove_by_url(url);
    char *url_total[2048] = { 0 };
    if (url[0] == '/') {
        strcat(url_total, "/event");
    } else {
        strcat(url_total, "/event/");
    }
    strcat(url_total, url);

    char *buf = NULL;
    int len = 0;
    coap_packet_t message[1];
    int mid = generate_mid();
    coap_init_message(message, COAP_TYPE_CON, COAP_DELETE, 0);
    coap_set_header_uri_path(message, url_total);
    coap_set_token(message, &mid, sizeof(mid));
    coap_set_header_content_format(message, 0);
    len = coap_serialize_message_tcp(message, &buf);

    char *imrt_link_message = NULL;
    int imrt_link_message_length = 0;
    imrt_link_message_encode(buf, len, &imrt_link_message,
            &imrt_link_message_length);
    ret = send_to_host_agent(imrt_link_message, imrt_link_message_length);

    free(buf);
    free(imrt_link_message);
}

void aee_request_init(aee_request_t *req, char *url, int action)
{
    if (NULL == req) {
        return;
    }
    memset(req, 0, sizeof(aee_request_t));
    req->action = action;
    req->url = url;
}

void aee_request_set_payload(aee_request_t *req, void *payload, int size,
        int fmt)
{
    if (fmt == PAYLOAD_FORMAT_ATTRIBUTE_OBJECT) {
        req->payload_fmt = fmt;
        char *buf = malloc(size);
        attr_container_serialize(buf, payload);
        req->payload = buf;
        req->payload_size = size;
    } else if (fmt
            == 98/* TODO: use FMT_APP_RAW_BINARY from shared_utils.h */) {
        req->payload_fmt = fmt;
        req->payload = payload;
        req->payload_size = size;
    }
}

void *aee_request_get_payload(aee_request_t *request, int *size, int *fmt)
{
    *size = request->payload_size;
    *fmt = request->payload_fmt;
    return request->payload;
}

#define URL_MAX_LEN 256
#define url_remain_space (sizeof(url) - strlen(url))

bool aee_applet_install(char *buf, char* module_type, int length,
        char *applet_name, int time_out)
{
    int ret;
    bool bl_ret = false;
    bool result;
    int app_type;
    struct timespec t;
    int err;
    char url[URL_MAX_LEN] = { 0 };

    snprintf(url, sizeof(url) - 1, "/applet?name=%s", applet_name);

    if (module_type != NULL && strlen(module_type) > 0)
        snprintf(url + strlen(url), url_remain_space, "&type=%s", module_type);

    /* TODO: set permissions AUDIO LOCATION SENSOR VISION platform.SERVICE */

    aee_request_t req;
    aee_request_init(&req, url, COAP_PUT);
    aee_request_set_payload(&req, buf, length,
            98/* TODO: use FMT_APP_RAW_BINARY from shared_utils.h */);

    //pthread_cond_init(&cond, NULL);
    clock_gettime(CLOCK_REALTIME, &t);

    ret = aee_request_send(&req, f_aee_install_applet_response_handler, &cond,
            time_out);
    if (ret) {
        t.tv_sec += time_out / 1000;
        //t.tv_nsec += (time_out % 1000000) * 1000000;
        pthread_mutex_lock(&mutex);
        err = pthread_cond_timedwait(&cond, &mutex, &t);
        pthread_mutex_unlock(&mutex);
        if (err == 0) {
            if (1 == flag_install_applet_success_flag) {
                bl_ret = true;
            } else {
                bl_ret = false;
            }
        } else {
            bl_ret = false;
        }
    } else {
        bl_ret = false;
    }
    transaction_remove_by_mid(req.mid);
    return bl_ret;
}

bool aee_applet_uninstall(char *applet_name, char* module_type, int time_out)
{
    int ret;
    bool bl_ret = false;
    bool result;
    char url[URL_MAX_LEN] = { 0 };

    snprintf(url, sizeof(url) - 1, "/applet?name=%s", applet_name);

    if (module_type != NULL && strlen(module_type) > 0)
        snprintf(url + strlen(url), url_remain_space, "&type=%s", module_type);

    struct timespec t;
    int err;

    aee_request_t req;
    aee_request_init(&req, url, COAP_DELETE);
    aee_request_set_payload(&req, NULL, 0, PAYLOAD_FORMAT_ATTRIBUTE_OBJECT);

    //pthread_cond_init(&cond, NULL);
    clock_gettime(CLOCK_REALTIME, &t);

    ret = aee_request_send(&req, f_aee_uninstall_applet_response_handler, &cond,
            time_out);
    if (ret) {
        pthread_mutex_lock(&mutex);
        t.tv_sec += time_out / 1000;
        //t.tv_nsec += (time_out % 1000000) * 1000000;
        err = pthread_cond_timedwait(&cond, &mutex, &t);
        if (err == 0) {
            if (1 == flag_uninstall_applet_success_flag) {
                bl_ret = true;
            } else {
                bl_ret = false;
            }
        } else {
            bl_ret = false;
        }
        pthread_mutex_unlock(&mutex);
    } else {
        bl_ret = false;
    }

    transaction_remove_by_mid(req.mid);
    return bl_ret;
}

bool aee_applet_list(int time_out, aee_applet_list_t *applet_list)
{
    bool ret = false;
    bool bl_ret = false;
    struct timespec t;
    int err;

    query_applet_usr_ctx usr_ctx;
    memset(&usr_ctx, 0, sizeof(query_applet_usr_ctx));

    attr_container_t *attr_obj = attr_container_create("test");
    attr_container_set_int(&attr_obj, "key1", 1000);
    aee_request_t req;
    aee_request_init(&req, "/applet", COAP_GET);
    //pthread_cond_init(&cond, NULL);
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += time_out / 1000;
    //t.tv_nsec += (time_out % 1000000) * 1000000;
    usr_ctx.condition = &cond;

    ret = aee_request_send(&req, f_aee_query_applet_response_handler, &usr_ctx,
            time_out);

    if (ret) {
        pthread_mutex_lock(&mutex);
        err = pthread_cond_timedwait(&cond, &mutex, &t);
        if (err == 0) {
            applet_list->cnt = usr_ctx.applets.cnt;
            applet_list->applets = usr_ctx.applets.applets;
            bl_ret = true;
        } else {
            bl_ret = false;
        }
        pthread_mutex_unlock(&mutex);
    } else {
        bl_ret = false;
    }
    transaction_remove_by_mid(req.mid);
    return bl_ret;
}

void aee_applet_list_init(aee_applet_list_t *applet_list)
{
    applet_list->cnt = 0;
    applet_list->applets = NULL;
}

void aee_applet_list_clean(aee_applet_list_t *applet_list)
{
    if (NULL != applet_list->applets) {
        free(applet_list->applets);
    }
    applet_list->cnt = 0;
}

const char *aee_applet_last_error()
{

    return NULL;
}

int send_to_host_agent(char *buf, int len)
{
    int total = 0;     // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;

    while (total < len) {
        pthread_mutex_lock(&mutex);
        n = send(sockfd, buf + total, bytesleft, 0);
        pthread_mutex_unlock(&mutex);
        if (n == -1) {
            break;
        }
        total += n;
        bytesleft -= n;
    }

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

void register_applet_exception_event()
{
}
void unregister_applet_exception_event()
{
}

int aee_response_get_status(aee_response_t *response)
{
    return response->status;
}

void *aee_response_get_payload(aee_response_t *response, int *size, int *fmt)
{
    *size = response->payload_size;
    *fmt = response->mid;
    return response->payload;
}
