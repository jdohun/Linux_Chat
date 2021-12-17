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
void* recvThread(void* data);		// 메시시 수신 함수
void* sendThreadClient();			// 클라이언트에 메시지를 보내는 함수

// 큐 관련 함수
int isFull();
int isEmpty();
int enqueue(Message item);
Message* dequeue();

int sock_main, sock_client[10];
Message* msgbuff;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int front = -1;
int rear = -1;

int main(int argc, char* argv[]) {
	int count = 0;
	int th_id;
	//Message buff;
	pthread_t th_send;

	if (argc != 2) {
		printf("Usege : %s [PORT]\n", argv[0]);
		exit(0);
	}
	printf("Server Open");

	struct sockaddr_in addr;
	memset(&addr, 0x00, sizeof(addr));

	pthread_t th_recv[10];	// client thread 식별자를 담을 배열

	msgbuff = (Message*)malloc(sizeof(Message) * MAX_BUFF);	// 메시지 정보를 담을 버퍼 동적할당

	// Create Send Thread
	th_id = pthread_create(&th_send, NULL, sendThread, 0);	// client로 부터 받은 
	if (th_id < 0) {
		printf("Send Thread Creation Failed\n");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(atoi(argv[1]));

	// ���� ���� IPv4 TCP
	if ((sock_main = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	// �ּ� ���ε�
	if (bind(sock_main, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Bind Failed\n");
		exit(2);
	}

	// listen
	if (listen(sock_main, 5) == -1) {
		printf("Listen Failed\n");
		exit(3);
	}

	while (1) {
		// accept and create client thread
		if ((sock_client[count] = accept(sock_main, NULL, NULL)) == -1) {
			printf("Accept Failed\n");
			continue;
		}
		else {
			if (count < 10) {
				Message* msgbuff;
				int idx = count;
				msgbuff->user_id = count;

				th_id = pthread_create(&th_recv[count], NULL, recvThread, (void*)msgbuff);
				if (th_id < 0) {
					printf("Receive Thread #%d Creation Failed\n", count + 1);
					exit(1);
				}
				count++;
			}
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

void* recvThread(void* data) {
	char name[20];
	Message *msgbuff = data;
	Message buff;
	buff.user_id = msgbuff->user_id;
	int thread_id = msgbuff->user_id;
	printf("Receive Thread %d Start\n", thread_id);
	
	// ������ �̽����� ť�� enqueue
	memset(&buff, 0, sizeof(Message));

	// 닉네임 설정
	/*
	printf("%s\n", "Enter a username (max 20 characters, no spaces):");
	fgets(name, sizeof(name), stdin);
	name[strlen(name) - 1] = '\0';
	*/

	while (recv(sock_client[thread_id], (char*)&buff, sizeof(buff), 0) > 0) {
		buff.user_id = thread_id;
		//strcpy(buff.username, name);	// 닉네임 복사 추가
		if (enqueue(buff) == -1) {
			printf("Message Buffer Full\n");
		}
	}
}

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
		strcpy(msgbuff[rear].username, item.username);	// 닉네임 복사 추가
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

/*
main 에서 매개 변수를 받아서 포트번호를 입력 받을 수 있도록 변경
message 구조체 안에 닉네임을 저장할 변수 생성
*/