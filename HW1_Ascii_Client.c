/** 컴퓨터네트워크(SW) 4분반 소프트웨어학과 32202970 윤예진
* 소켓프로그래밍 과제 1 : 문자 --> 아스키 코드값 통신 (클라이언트)
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#define BUF_SIZE 256


// 서버에 최대 5byte 문자열을 보내면 아스키 코드값(정수)을 수신
void main(void)
{
    int cs;                                 //클라이언트 소켓
    char buf[BUF_SIZE] = {0, };
    int size = 0;

    // 소켓 주소 설정 및 초기화
    struct sockaddr_in csa;
    memset(&csa, 0, sizeof(csa));
    csa.sin_family = AF_INET;
    csa.sin_addr.s_addr = htonl(INADDR_ANY);
    csa.sin_port = htons(13131);

    // 소켓 생성 및 바인딩
    cs = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect(cs, (struct sockaddr *) &csa, sizeof(csa))<0)
    {
        printf("Failed to connect\n");
        close(cs);
        exit(1);
    }

    // 사용자 입력
    printf("Enter maximum 5 char : ");
    scanf("%s", buf);
    size = strlen(buf);
    printf("size: %d\n", size);
    // 5글자 초과
    if(size > 5) {
        printf("Over 5 char\n");
        close(cs);
        exit(1);
    }

    // 전송
    if(send(cs, buf, BUF_SIZE, 0) < BUF_SIZE)
    {
        printf("Send Error!\n");
    }

    // 수신 후 출력
    recv(cs, buf, BUF_SIZE, 0);
    for(int i=0; i<size; i++)
    {
        printf("%d ", ((int*)buf)[i]);
    }
    close(cs);
}