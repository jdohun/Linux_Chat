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

static pthread_t snd_thr, recv_thr;   // pthread 자료형을 담을 변수
static int thr_exit = TRUE;    // thread 출구의 상태 : 열림
static int thr_id;              // tread의 id를 저장할 변수
static void* treturn;           // tread return
static struct sockaddr_in server_addr, client_addr; // server의 주소와 client의 주소를 담을 구조체 객체
static int server_fd, client_fd, n, n2;     // 서버, 클라이언트 소켓과 송수신 상태를 저장할 변수
static char recv_data[BUF_LEN]; // 수신한 데이터를 저장할 버퍼
char msg[BUF_LEN];             // msg

void* thread_recv(void* arg);
void* send_msg(void* arg);

void thread_start(int sock) {
    thr_exit = FALSE;
    pthread_create(&recv_thr, NULL, thread_recv, (void*)&sock);  // client 소켓으로부터 정보를 수신받는 pthread 생성
    pthread_create(&snd_thr, NULL, send_msg, (void*)&sock);  // client 소켓으로부터 정보를 수신받는 pthread 생성
}

void thread_stop() {
    thr_exit = TRUE;
    pthread_join(snd_thr, &treturn); // thread가 종료하길 기다리는 함수, 성공하면 0 반환, 실패하면 에러코드 반환
    pthread_join(recv_thr, &treturn); // thread가 종료하길 기다리는 함수, 성공하면 0 반환, 실패하면 에러코드 반환
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

int main(int argc, char* argv[]) {
    char chat_data[BUF_LEN];    // 채팅 내용을 담을 버퍼
    char temp[20];  // 주소 정보를 임시 저장할 버퍼
    int len;        // 수신한 정보 길이를 저장할 변수
    int sock;

    if (argc != 2) {    // 매개 변수가 2개가 안되면 오류 발생
        printf("Usege ./filename [PORT] \n");
        exit(0);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {  // 서버 소켓 생성 시 반환 값이 -1이면 오류 발생
        printf("Server: can not Open Socket\n");
        exit(0);
    }

    memset(&server_addr, 0x00, sizeof(server_addr));    // server 소켓의 메모리 버퍼 0으로 초기화

    server_addr.sin_family = AF_INET;   // 주소 체계 필수 고정 internet
                                       // : NADDR_ANY는 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network Long : 32비트 IP주소
    server_addr.sin_port = htons(atoi(argv[1]));        // Host to Network Short : 16비트 포트 번호 : main 매개변수

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { // 서버 소켓을 인터넷과 연결한 소켓에 등록
        printf("Server: cat not bind local addrss\n");
        exit(0);
    }

    if (listen(sock, 5) < 0) { // 백로그 큐에 최대 5개의 요청을 저장 : 성공시 0 반환
        printf("Server: cat not listen connnect.\n");
        exit(0);
    }

    memset(recv_data, 0x00, sizeof(recv_data));     // 수신 버퍼 0 초기화
    len = sizeof(sock);  // 수신한 정보의 길이 측정
    printf("=====[PORT] : %d =====\n", atoi(argv[1]));  // ASCII to Intager : 포트번호 출력
    printf("Server : wating connection request.\n");    // 

    while (1) {
        client_fd = accept(sock, (struct sockaddr*)&client_addr, (socklen_t*)&len);    // 서버에 클라이언트 접근

        if (client_fd < 0) {    // 접근 실패
            printf("Server: accept failed\n");
            exit(0);
        }
        
        // IPv4 와 IPv6 주소를 binary 형태에서 사람이 알아보기 쉬운 텍스트(human-readable text)형태로 전환
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));  // 

        printf("Server: %s client connect.\n", temp);   // 서버 연결 됨
        printf("\n%s(%d)님이 들어오셨습니다. 나가려면 (quit)을 누르세요\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));  // Network to ASCII, Network to Host Short

        while (1) {
            thread_start(sock); // thread 시작
            fgets(chat_data, sizeof(chat_data), stdin); // 입력 받은 정보를 저장
            if ((n2 = send(client_fd, chat_data, sizeof(chat_data), 0)) == -1) { // 정보 전송
                break;
            }
        }

        thread_stop();  // thread 정지
        close(client_fd);
        printf("Server: %s client closed.\n", temp);
    }

    close(server_fd);

    return 0;
}