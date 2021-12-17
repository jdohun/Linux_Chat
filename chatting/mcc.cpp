#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0
#define BUF_SIZE 1024

static pthread_t thr;
static int thr_id;
static int thr_exit = TRUE;
static int client_fd, len, n, n2;
static void* treturn;

void* thread_recv(void* arg);

void thread_start(void *arg) {
    thr_exit = FALSE;
    thr_id = pthread_create(&thr, NULL, thread_recv, arg);
}

void thread_stop() {
    thr_exit = TRUE;
    thr_id = pthread_join(thr, &treturn);
}

void* thread_recv(void* arg) {
    int client_fd = (void *)((int)*arg);
    char recv_data[BUF_SIZE];
    while (!thr_exit) {
        memset(recv_data, 0x00, sizeof(recv_data));
        if ((n = read(client_fd, recv_data, sizeof(recv_data))) == -1) {
            printf("disconnect!!\n");
            return (int*)0;
        }
        else if (n > 0) {
            recv_data[n] = '\0';
            printf("\n[관리자]: %s\n", recv_data);
        }
    }
    pthread_exit((void*)0);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in client_addr;
    char* IP = argv[1];
    in_port_t PORT = atoi(argv[2]);
    char chat_data[BUF_SIZE];

    if (argc != 3) {
        printf("Usege : ./%s [IP] [PORT]\n",argv[0]);
        exit(0);
    }

    client_fd = socket(PF_INET, SOCK_STREAM, 0);

    client_addr.sin_addr.s_addr = inet_addr(IP);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);

    if (connect(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        printf("Can't connect\n");
        close(client_fd);
        return -1;
    }

    thread_start((void *)(&client_fd));
    while (1) {
        fgets(chat_data, sizeof(chat_data), stdin);
        send(client_fd, chat_data, sizeof(chat_data), 0);
    }

    thread_stop();

    close(client_fd);

    return 0;
}