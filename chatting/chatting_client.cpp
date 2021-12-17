#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0
#define BUF_SIZE 1024

static pthread_t sThr, rThr;        // pthread �ڷ����� ���� ����
static int thr_sId, thr_rId;        // pthread�� id�� ������ ����, ���� ���� thread, ���� ���� thread
static int thr_exit = TRUE;         // pthread�� �ⱸ ����
static char recv_data[BUF_SIZE];    // ���޹��� ������ ������ ����
static int client_fd, len, n;       // ù��° cllient ������ ����, ���� ����, ���� ���� ����
static int client_fd2, len2, n2;    // �ι�° cllient ������ ����, ���� ����, ���� ���� ����
static void* sTreturn, *rTreturn;    // thread return 
char msg[BUF_SIZE];             // msg

void* thread_recv(void* arg);
void* thread_send(void* arg);

void thread_start(int socket) {
    thr_exit = FALSE;
    thr_sId = pthread_create(&sThr, NULL, thread_send, (void*)&socket);        // client �������� ������ �����ϴ� pthread ����
    thr_rId = pthread_create(&rThr, NULL, thread_recv, (void*)&socket); // client �������κ��� ������ ���Ź޴� pthread ����
}

void thread_stop() {
    thr_exit = TRUE;
    pthread_join(sThr, &sTreturn);   // thread�� �����ϱ� ��ٸ��� �Լ�, �����ϸ� 0 ��ȯ, �����ϸ� �����ڵ� ��ȯ
    pthread_join(rThr, &rTreturn);
}

void* thread_recv(void* arg) {
    while (thr_exit == FALSE) {
        if ((n = read(client_fd, recv_data, sizeof(recv_data))) == -1) { // client �������κ��� ���Ź��� ������ -1 �̸� ���� X
            printf("disconnect!!\n");
            return (int*)0;
        }
        else if (n > 0) {
            recv_data[n] += '\0'; // ���ڿ��� ���� ǥ���ϱ� ���� �ι��� �߰�
            printf("\n[������]: %s\n", recv_data);
            //return (int*)0;
        }
    }
    pthread_exit((void*)0);// Thread ����
}

void* thread_send(void* arg) {
    int sock = *((int*)arg);
    char name_msg[BUF_SIZE];
    char myInfo[BUF_SIZE];
    char* who = NULL;
    char temp[BUF_SIZE];

    while (1) {
        fgets(msg, BUF_SIZE, stdin);

        if (!strcmp(msg, "!menu\n")) {
            continue;
        }

        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            close(sock);
            exit(0);
        }

        // send message
        sprintf(name_msg, "%s", msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    struct sockaddr_in client_addr;
    char* IP = argv[1];             // �Ű� ����1�� ip�� ����
    in_port_t PORT = atoi(argv[2]); // �Ű� ����2�� ��Ʈ��ȣ�� ����
    char chat_sData[BUF_SIZE];       // ä�� ������ ������ ����

    if (argc != 3) {       // �Ű����� 3�� �����̸� ����
        printf("Usege : ./filename [IP] [PORT] \n");
        exit(0);
    }
    // �������� ü�� : IPv4���ͳ� �������� 
    client_fd = socket(PF_INET, SOCK_STREAM, 0); // client ���� ����

    client_addr.sin_addr.s_addr = inet_addr(IP);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);

    if (connect(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        printf("Can't connect\n");
        close(client_fd);
        return -1;
    }

    while (1) {
        thread_start(client_fd);
        fgets(chat_sData, sizeof(chat_sData), stdin);
        send(client_fd, chat_sData, sizeof(chat_sData), 0);
    }
    thread_stop();

    close(client_fd);
    return 0;
}
