/** 컴퓨터네트워크(SW) 4분반 소프트웨어학과 32202970 윤예진
* 소켓프로그래밍 과제 1 : 문자 --> 아스키 코드값 통신 (서버)
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#define BUF_SIZE 256


// 클라이언트로부터 문자를 받아 아스키 코드값(정수)으로 변환 후 전송
void main(void)
{
    int ss = -1;                        // server socket
    int sa = -1;                        // socket accepted
    char buf[BUF_SIZE];                 // 통신 버퍼
    int size = 0;                       // 문자열 길이
    char* asciiStr = NULL;              // 클라이언트가 보낸 아스키 문자열
    int asciiInt = 0;                   // 아스키 코드값

    // 소켓 주소 설정 및 초기화
    struct sockaddr_in ssa;
    memset(&ssa, 0, sizeof(ssa));
    ssa.sin_family = AF_INET;                           // IPv4 주소 체계 설정
    ssa.sin_addr.s_addr = htonl(INADDR_ANY);
    ssa.sin_port = htons(13131);

    ss = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);             // 소켓 생성
    if(bind(ss, (struct sockaddr *) &ssa, sizeof(ssa)) < 0)    // 소켓을 포트와 연결
    {
        printf("failed to bind socket\n");
        close(ss);
        exit(1);
    }

    // 요청 대기
    listen(ss, 10);
    sa = accept(ss, 0, 0);

    // 클라이언트로부터 데이터 수신
    recv(sa, buf, BUF_SIZE, 0);

    size = strlen(buf);
    asciiStr = (char*)malloc(sizeof(char)*size);
    strncpy(asciiStr, buf, size);
    for(int i=0; i<size; i++) {
        asciiInt = asciiStr[i];                     //각 문자의 아스키 코드 값(정수)
        ((int*)buf)[i] = asciiInt;
    }

    // 정수 아스키 코드 값 전송
    send(sa, buf, BUF_SIZE, 0);
    close(sa);
}