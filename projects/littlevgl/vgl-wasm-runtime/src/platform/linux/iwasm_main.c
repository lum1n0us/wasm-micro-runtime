#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

#include "runtime_lib.h"
#include "runtime_timer.h"
#include "native_interface.h"
#include "app-manager-export.h"
#include "bh_common.h"
#include "bh_queue.h"
#include "bh_thread.h"
#include "bh_memory.h"
#include "runtime_sensor.h"
#include "attr-container.h"
#include "module_wasm_app.h"
#include "wasm-export.h"

extern void * thread_timer_check(void *);
extern void init_sensor_framework();
extern int aee_host_msg_callback(void *msg, uint16_t msg_len);

#ifndef __ZEPHYR__
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX 80
#define PORT 8888
#define SA struct sockaddr

int listenfd = -1;
int sockfd = -1;

static bool server_mode = false;
static pthread_mutex_t sock_lock = PTHREAD_MUTEX_INITIALIZER;

// Function designed for chat between client and server.
void* func(void* arg)
{
    char buff[MAX];
    int n;
    int connfd, len;
    struct sockaddr_in servaddr, cli;

    while (1) {
        if (sockfd != -1)
            close(sockfd);
        // socket create and verification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            return NULL;
        } else
            printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));
        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        servaddr.sin_port = htons(PORT);

        // connect the client socket to server socket
        if (connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            sleep(10);
            continue;
        } else {
            printf("connected to the server..\n");

            host_send("hello", 6);
        }

        // infinite loop for chat
        for (;;) {
            bzero(buff, MAX);

            // read the message from client and copy it in buffer
            n = read(sockfd, buff, sizeof(buff));
            // print buffer which contains the client contents
            //fprintf(stderr, "recieved %d bytes from host: %s", n, buff);

            // socket disconnected
            if (n <= 0)
                break;

            aee_host_msg_callback(buff, n);
        }
    }

    // After chatting close the socket
    close(sockfd);
}

static bool host_init()
{
    return true;
}

int host_recv(void * ctx, char *buf, int buf_size)
{
    return 0;
}

int host_send(void * ctx, const char *buf, int size)
{
    int ret;

    if (pthread_mutex_trylock(&sock_lock) == 0) {
        if (sockfd == -1) {
            pthread_mutex_unlock(&sock_lock);
            return 0;
        }

        ret = write(sockfd, buf, size);

        pthread_mutex_unlock(&sock_lock);
        return ret;
    }

    return -1;
}

void host_destroy()
{
    if (server_mode)
        close(listenfd);

    pthread_mutex_lock(&sock_lock);
    close(sockfd);
    pthread_mutex_unlock(&sock_lock);
}
void* func_server_mode(void* arg)
{
    int newsockfd, portno, clilent;
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;
    char buff[MAX];

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    /* First call to socket() function */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 8866;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(listenfd, 5);
    clilent = sizeof(cli_addr);

    while (1) {
        pthread_mutex_lock(&sock_lock);

        sockfd = accept(listenfd, (struct sockaddr *) &cli_addr, &clilent);

        pthread_mutex_unlock(&sock_lock);

        if (sockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        printf("connection established!\n");

        for (;;) {
            bzero(buff, MAX);

            // read the message from client and copy it in buffer
            n = read(sockfd, buff, sizeof(buff));

            // socket disconnected
            if (n <= 0) {
                pthread_mutex_lock(&sock_lock);
                close(sockfd);
                sockfd = -1;
                pthread_mutex_unlock(&sock_lock);

                printf(
                        "connection lost, and waiting for client to reconnect...\n");
                sleep(2);
                break;
            }

            aee_host_msg_callback(buff, n);
        }
    }
}

extern void display_SDL_init();
// Driver function

#else

#include <zephyr.h>
#include <uart.h>
#include <device.h>

#define HOST_LMT_COMM_UART_NAME "UART_1"
int uart_char_cnt = 0;

static void uart_irq_callback(struct device *dev)
{
    unsigned char ch;
    int size = 0;

    while (uart_poll_in(dev, &ch) == 0) {

        uart_char_cnt++;
        aee_host_msg_callback(&ch, 1);
    }

    /*app_manager_printf("Uart IRQ: total %d bytes received\n", size);*/
}

struct device *uart_dev = NULL;

static bool host_init()
{
    uart_dev = device_get_binding(HOST_LMT_COMM_UART_NAME);
    if (!uart_dev) {
        printf("UART: Device driver not found.\n");
        return;
    }
    uart_irq_rx_enable(uart_dev);
    uart_irq_callback_set(uart_dev, uart_irq_callback);
    return true;
}

int host_recv(void * ctx, char *buf, int buf_size)
{
    return 0;
}

int host_send(void * ctx, const char *buf, int size)
{
    for (int i = 0; i < size; i++)
    uart_poll_out(uart_dev, buf[i]);

    return size;
}

void host_destroy()
{

}

#endif

#define DEFAULT_THREAD_STACKSIZE (8 * 1024)

host_interface interface = { .init = host_init, .recv = host_recv, .send =
        host_send, .destroy = host_destroy };
timer_ctx_t timer_ctx;

static char global_heap_buf[1024 * 1024] = { 0 };

int iwasm_main(int argc, char *argv[])
{
    korp_thread tid, tm_tid;

    host_init();

    if (bh_memory_init_with_pool(global_heap_buf, sizeof(global_heap_buf))
            != 0) {
        printf("Init global heap failed.\n");
        return -1;
    }

    if (vm_thread_sys_init() != 0) {
        goto fail1;
    }

    if (argc == 2 && strcmp(argv[1], "-s") == 0)
        server_mode = true;
#ifndef __ZEPHYR__
    extern void display_SDL_init();
    display_SDL_init();
#else
    extern void display_init(void);
    display_init();
#endif

    // timer manager
    init_wasm_timer();

#ifndef __ZEPHYR__
    if (server_mode)
        vm_thread_create(&tid, func_server_mode, NULL,
        BH_APPLET_PRESERVED_STACK_SIZE);
    else
        vm_thread_create(&tid, func, NULL, BH_APPLET_PRESERVED_STACK_SIZE);
#endif

    // TODO:
    app_manager_startup(&interface);

    fail1: bh_memory_destroy();
    return -1;
}
