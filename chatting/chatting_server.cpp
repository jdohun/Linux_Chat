#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define TRUE 1
#define FALSE 0
#define NORMAL_LEN 20
#define BUF_LEN 1024

static pthread_t snd_thr, recv_thr;   // pthread �ڷ����� ���� ����
static int thr_exit = TRUE;    // thread �ⱸ�� ���� : ����
static int thr_id;              // tread�� id�� ������ ����
static void* treturn;           // tread return
static struct sockaddr_in server_addr, client_addr; // server�� �ּҿ� client�� �ּҸ� ���� ����ü ��ü
static int server_fd, client_fd, n, n2;     // ����, Ŭ���̾�Ʈ ���ϰ� �ۼ��� ���¸� ������ ����
static char recv_data[BUF_LEN]; // ������ �����͸� ������ ����
char msg[BUF_LEN];             // msg

void* thread_recv(void* arg);
void* send_msg(void* arg);

void thread_start(int sock) {
    thr_exit = FALSE;
    pthread_create(&recv_thr, NULL, thread_recv, (void*)&sock);  // client �������κ��� ������ ���Ź޴� pthread ����
    pthread_create(&snd_thr, NULL, send_msg, (void*)&sock);  // client �������κ��� ������ ���Ź޴� pthread ����
}

void thread_stop() {
    thr_exit = TRUE;
    pthread_join(snd_thr, &treturn); // thread�� �����ϱ� ��ٸ��� �Լ�, �����ϸ� 0 ��ȯ, �����ϸ� �����ڵ� ��ȯ
    pthread_join(recv_thr, &treturn); // thread�� �����ϱ� ��ٸ��� �Լ�, �����ϸ� 0 ��ȯ, �����ϸ� �����ڵ� ��ȯ
}

void* send_msg(void* arg) {
    int sock = *((int*)arg);
    char name_msg[NORMAL_LEN + BUF_LEN];
    char myInfo[BUF_LEN];
    char* who = NULL;
    char temp[BUF_LEN];

    while (1) {
        fgets(msg, BUF_LEN, stdin);

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

void* thread_recv(void* arg) {
    while (thr_exit == FALSE) { // �ⱸ�� ����������
        if ((n = recv(client_fd, recv_data, sizeof(recv_data), 0)) == -1) { // client �������κ��� ���� X
            printf("[������: %s(%d)] disconnect\n", inet_ntoa(client_addr.sin_addr)
                , ntohs(client_addr.sin_port)); // Network �̸�, �ּ� ���
            thread_stop();
            close(client_fd);
        }
        else if (n > 0) {
            recv_data[n] = '\0'; // ������ ���ڿ� null���� �߰� : ������ ���� �� �� �ֵ���
            printf("\n[�����ڴ�: %s(%d)]: %s\n", inet_ntoa(client_addr.sin_addr) // Network to ASCII
                , ntohs(client_addr.sin_port), recv_data);  // Network to Host Short
            // Network �̸�, �ּ�, ���� ���� ���
        }
    }
    pthread_exit((void*)0); // Thread ����
}

int main(int argc, char* argv[]) {
    char chat_data[BUF_LEN];    // ä�� ������ ���� ����
    char temp[20];  // �ּ� ������ �ӽ� ������ ����
    int len;        // ������ ���� ���̸� ������ ����
    int sock;

    if (argc != 2) {    // �Ű� ������ 2���� �ȵǸ� ���� �߻�
        printf("Usege ./filename [PORT] \n");
        exit(0);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {  // ���� ���� ���� �� ��ȯ ���� -1�̸� ���� �߻�
        printf("Server: can not Open Socket\n");
        exit(0);
    }

    memset(&server_addr, 0x00, sizeof(server_addr));    // server ������ �޸� ���� 0���� �ʱ�ȭ

    server_addr.sin_family = AF_INET;   // �ּ� ü�� �ʼ� ���� internet
                                       // : NADDR_ANY�� �ڵ����� �� ��ǻ�Ϳ� �����ϴ� ��ī�� �� ��밡���� ��ī���� IP�ּҸ� ����϶�� �ǹ�
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network Long : 32��Ʈ IP�ּ�
    server_addr.sin_port = htons(atoi(argv[1]));        // Host to Network Short : 16��Ʈ ��Ʈ ��ȣ : main �Ű�����

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { // ���� ������ ���ͳݰ� ������ ���Ͽ� ���
        printf("Server: cat not bind local addrss\n");
        exit(0);
    }

    if (listen(sock, 5) < 0) { // ��α� ť�� �ִ� 5���� ��û�� ���� : ������ 0 ��ȯ
        printf("Server: cat not listen connnect.\n");
        exit(0);
    }

    memset(recv_data, 0x00, sizeof(recv_data));     // ���� ���� 0 �ʱ�ȭ
    len = sizeof(sock);  // ������ ������ ���� ����
    printf("=====[PORT] : %d =====\n", atoi(argv[1]));  // ASCII to Intager : ��Ʈ��ȣ ���
    printf("Server : wating connection request.\n");    // 

    while (1) {
        client_fd = accept(sock, (struct sockaddr*)&client_addr, (socklen_t*)&len);    // ������ Ŭ���̾�Ʈ ����

        if (client_fd < 0) {    // ���� ����
            printf("Server: accept failed\n");
            exit(0);
        }
        
        // IPv4 �� IPv6 �ּҸ� binary ���¿��� ����� �˾ƺ��� ���� �ؽ�Ʈ(human-readable text)���·� ��ȯ
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));  // 

        printf("Server: %s client connect.\n", temp);   // ���� ���� ��
        printf("\n%s(%d)���� �����̽��ϴ�. �������� (quit)�� ��������\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));  // Network to ASCII, Network to Host Short

        while (1) {
            thread_start(sock); // thread ����
            fgets(chat_data, sizeof(chat_data), stdin); // �Է� ���� ������ ����
            if ((n2 = send(client_fd, chat_data, sizeof(chat_data), 0)) == -1) { // ���� ����
                break;
            }
        }

        thread_stop();  // thread ����
        close(client_fd);
        printf("Server: %s client closed.\n", temp);
    }

    close(server_fd);

    return 0;
}