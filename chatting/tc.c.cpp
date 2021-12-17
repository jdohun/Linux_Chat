#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<pthread.h>
#include<arpa/inet.h>

#define TRUE 1
#define FALSE 0
#define BUF_SIZE 1024
#define NORMAL_SIZE 20

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
void thread_start();
void thread_stop();

static int thr_exit = TRUE;     // pthread의 출구 상태
static pthread_t snd_thread, rcv_thread;
static void* thread_return;

char msg_form[NORMAL_SIZE];     // msg form
char msg[BUF_SIZE];             // msg
char serv_port[NORMAL_SIZE];    // server port number
char clnt_ip[NORMAL_SIZE];      // client ip address

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char* IP = argv[1];             // 매개 변수1을 ip로 저장
    in_port_t PORT = atoi(argv[2]); // 매개 변수2를 포트번호로 저장

    if (argc != 3) {
        printf(" Usage : %s [IP] [PORT]\n", argv[0]);
        exit(1);
    }

    // 프로토콜 체계 : IPv4인터넷 프로토콜
    sock = socket(PF_INET, SOCK_STREAM, 0); // client 소켓 생성

    memset(&serv_addr, 0, sizeof(serv_addr));    // 소켓 주소 메모리 초기화
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("Can't connect\n");
        close(sock);
        return -1;
    }

    while (1) {
        thread_start(sock);
        fgets(msg, sizeof(msg), stdin);
        send(sock, msg, sizeof(msg), 0);
    }
    thread_stop();

    close(sock);
    return 0;
}

void thread_start(int sock) {
    thr_exit = FALSE;
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
}

void thread_stop() {
    thr_exit = TRUE;
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
}

void* send_msg(void* arg) {
    int sock = *((int*)arg);
    char name_msg[NORMAL_SIZE + BUF_SIZE];
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

void* recv_msg(void* arg) {
    int sock = *((int*)arg);
    char name_msg[NORMAL_SIZE + BUF_SIZE];
    int str_len;

    while (thr_exit == FALSE) {
        str_len = read(sock, name_msg, NORMAL_SIZE + BUF_SIZE - 1);
        if (str_len == -1)
            return (void*)-1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}
















void* thread_recv(void* arg) {
    while (thr_exit == FALSE) { // 출구가 닫혀있으면
        if ((n = recv(client_fd, recv_data, sizeof(recv_data), 0)) == -1) { // client 소켓으로부터 수신 X
            printf("[접속자: %s(%d)] disconnect\n", inet_ntoa(client_addr.sin_addr)
                , ntohs(client_addr.sin_port)); // Network 이름, 주소 출력
            thread_stop();
            close(client_fd);
        }
        else if (n > 0) {
            recv_data[n] = '\0'; // 마지막 글자에 null문자 추가 : 문장의 끝을 알 수 있도록
            printf("\n[접속자님: %s(%d)]: %s\n", inet_ntoa(client_addr.sin_addr) // Network to ASCII
                , ntohs(client_addr.sin_port), recv_data);  // Network to Host Short
            // Network 이름, 주소, 받은 정보 출력
        }
    }
    pthread_exit((void*)0); // Thread 종료
}