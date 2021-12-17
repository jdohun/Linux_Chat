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

static pthread_t sThr, rThr;        // pthread 자료형을 담을 변수
static int thr_sId, thr_rId;        // pthread의 id를 저장할 변수, 전송 전용 thread, 수신 전용 thread
static int thr_exit = TRUE;         // pthread의 출구 상태
static char recv_data[BUF_SIZE];    // 전달받은 정보를 저장할 버퍼
static int client_fd, len, n;       // 첫번째 cllient 소켓을 저장, 길이 저장, 전송 상태 저장
static int client_fd2, len2, n2;    // 두번째 cllient 소켓을 저장, 길이 저장, 수신 상태 저장
static void* sTreturn, *rTreturn;    // thread return 
char msg[BUF_SIZE];             // msg

void* thread_recv(void* arg);
void* thread_send(void* arg);

void thread_start(int socket) {
    thr_exit = FALSE;
    thr_sId = pthread_create(&sThr, NULL, thread_send, (void*)&socket);        // client 소켓으로 정보를 전송하는 pthread 생성
    thr_rId = pthread_create(&rThr, NULL, thread_recv, (void*)&socket); // client 소켓으로부터 정보를 수신받는 pthread 생성
}

void thread_stop() {
    thr_exit = TRUE;
    pthread_join(sThr, &sTreturn);   // thread가 종료하길 기다리는 함수, 성공하면 0 반환, 실패하면 에러코드 반환
    pthread_join(rThr, &rTreturn);
}

void* thread_recv(void* arg) {
    while (thr_exit == FALSE) {
        if ((n = read(client_fd, recv_data, sizeof(recv_data))) == -1) { // client 소켓으로부터 수신받은 정보가 -1 이면 연결 X
            printf("disconnect!!\n");
            return (int*)0;
        }
        else if (n > 0) {
            recv_data[n] += '\0'; // 문자열의 끝을 표시하기 위해 널문자 추가
            printf("\n[관리자]: %s\n", recv_data);
            //return (int*)0;
        }
    }
    pthread_exit((void*)0);// Thread 종료
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
    char* IP = argv[1];             // 매개 변수1을 ip로 저장
    in_port_t PORT = atoi(argv[2]); // 매개 변수2를 포트번호로 저장
    char chat_sData[BUF_SIZE];       // 채팅 내용을 전달할 버퍼

    if (argc != 3) {       // 매개변수 3개 이하이면 종료
        printf("Usege : ./filename [IP] [PORT] \n");
        exit(0);
    }
    // 프로토콜 체계 : IPv4인터넷 프로토콜 
    client_fd = socket(PF_INET, SOCK_STREAM, 0); // client 소켓 생성

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
