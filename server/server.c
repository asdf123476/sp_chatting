#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void send_msg(char *msg, int len, int clnt_num); // 메시지 전송 함수

int clnt_cnt = 0;         // 클라이언트 소켓이 접속한 수
int clnt_socks[MAX_CLNT]; // 클라이언트 소켓들 저장
pthread_mutex_t mutx;     // 뮤텍스

void *handle_clnt(void *arg) // 쓰레드가 처리할 함수
{
    int clnt_sock = *((int *)arg); // 클라이언트 소켓
    int str_len = 0, i;
    char msg[BUF_SIZE];

    while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) // 클라이언트에게 메시지를 받음
        send_msg(msg, str_len, clnt_sock);

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) // 클라이언트 연결 해제 과정
    {
        if (clnt_sock == clnt_socks[i])
        {
            while (i < clnt_cnt - 1)
            {
                clnt_socks[i] = clnt_socks[i + 1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}
void send_msg(char *msg, int len, int clnt_num) // 클라이언트들에게 메시지 전송
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)     // 클라이언트 수 만큼 메시지 전송
        if (clnt_num != clnt_socks[i]) // 전송한 클라이언트는 제외
            write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutx);
}

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);             // 뮤텍스 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); // 소켓 생성

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET; // 주소 설정
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) // 소켓 연결
    {
        perror("bind() error");
        exit(1);
    }
    if (listen(serv_sock, 5) == -1)
    {
        perror("listen() error");
        exit(1);
    }

    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz); // 연결 허용

        pthread_mutex_lock(&mutx);          // 뮤텍스 획득
        clnt_socks[clnt_cnt++] = clnt_sock; // 클라이언트 소켓 저장
        pthread_mutex_unlock(&mutx);        // 뮤텍스 반환

        pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock); // 쓰레드 생성
        pthread_detach(t_id);                                         // 쓰레드 소멸
        printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    return 0;
}