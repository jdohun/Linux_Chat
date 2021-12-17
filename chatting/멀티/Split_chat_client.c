/// Linux
// ������ �ɼ� -pthread �߰�
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <pthread.h>

#define MAX_BUFF 100
#define USERNAME_MAX_SIZE 20
#define MAX_MESSAGE_LEN 256

typedef struct Message {
	int user_id;
	char username[USERNAME_MAX_SIZE];
	char str[MAX_MESSAGE_LEN];
} Message;

void* sendThread();					// 메시지 송신 함수
void* sendThreadClient();			// 서버를 통해 클라이언트에 메시지를 보내는 함수
// void* recvThread(void* data);		// 메시지 수신 함수

// 큐 관련 함수
int isFull();
int isEmpty();
int enqueue(Message item);
Message* dequeue();

int sock_main, sock_client[10];		// 메인
Message* msgbuff;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int front = -1;
int rear = -1;

int main(int argc, char* argv[]) {
	int count = 0;				// client의 수량과 동시에 id
	int th_id;					// thread 생성 성공 여부 식별자
	Message* buff;				// 전달할 내용이 담길 구조체
	pthread_t th_send;			// thread가 생성될 곳

	if (argc != 3) {
		printf("Usege : %s [IP] [PORT]\n", argv[0]);
		exit(0);
	}

	struct sockaddr_in addr;			// thread가 연결될 socket

	char* IP = argv[1];					// IP
	in_port_t PORT = atoi(argv[2]);		// PORT

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(IP);

	// ���� ���� IPv4 TCP
	if ((sock_main = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	// Connect
	if (connect(sock_main, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Connect Failed\n");
		exit(4);
	}

	// Client Send Thread
	th_id = pthread_create(&th_send, NULL, sendThreadClient, 0);

	if (th_id < 0) {
		printf("Send Thread Creation Failed\n");
		exit(1);
	}

	while (1) {
		// �޽��� ���� �� ���
		memset(&buff, 0, sizeof(buff));
		if (recv(sock_main, (void *)buff, sizeof(buff), 0) > 0) {
			printf("User %s: %s\n", buff->username, buff->str);
		}
		else {
			printf("Disconnected\n");
			exit(5);
		}
	}
	return 0;
}

void* sendThread() {
	Message* tmp;

	printf("Send Thread Start\n");

	while (1) {
		// ť���� ������ �޽����� ������ dequeue �� ������ �������� ����
		if ((tmp = dequeue()) != NULL) {
			for (int i = 0; i < 10; i++) {
				if (i != tmp->user_id) {	// 자기 자신을 제외한 나머지에게 메시지 전송
					send(sock_client[i], (char*)tmp, sizeof(Message), 0);
				}
			}
		}
		usleep(1000);
	}
}

/*
void* recvThread(void* data) {
	Message buff;
	int thread_id = *((int*)data);

	printf("Receive Thread %d Start\n", thread_id);

	// ������ �̽����� ť�� enqueue
	memset(&buff, 0, sizeof(Message));

	while (recv(sock_client[thread_id], (char*)&buff, sizeof(buff), 0) > 0) {
		buff.user_id = thread_id;
		if (enqueue(buff) == -1) {
			printf("Messag Buffer Full\n");
		}
	}
}
*/

void* sendThreadClient() {
	Message tmp;
	int count;

	while (1) {
		// �޽����� �Է� ���� �� ����
		memset(&tmp, 0, sizeof(tmp));
		//scanf("%[^\n]s", tmp.str);
		fgets(tmp.str, MAX_MESSAGE_LEN, stdin);
		tmp.str[strlen(tmp.str) - 1] = '\0';
		tmp.user_id = -1;

		count = send(sock_main, (char*)&tmp, sizeof(Message), 0);
	}
}

int isFull() {
	if ((front == rear + 1) || (front == 0 && rear == MAX_BUFF - 1)) {
		return 1;
	}
	return 0;
}

int isEmpty() {
	if (front == -1) {
		return 1;
	}
	return 0;
}

int enqueue(Message item) {
	if (isFull()) {
		return -1;
	}
	else {
		pthread_mutex_lock(&mutex);
		if (front == -1) {
			front = 0;
		}
		rear = (rear + 1) % MAX_BUFF;
		msgbuff[rear].user_id = item.user_id;
		strcpy(msgbuff[rear].str, item.str);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}

Message* dequeue() {
	Message* item;

	if (isEmpty()) {
		return NULL;
	}
	else {
		pthread_mutex_lock(&mutex);
		item = &msgbuff[front];

		if (front == rear) {
			front = -1;
			rear = -1;
		}
		else {
			front = (front + 1) % MAX_BUFF;
		}
		pthread_mutex_unlock(&mutex);
		return item;
	}
}