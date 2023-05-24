/** 컴퓨터네트워크(SW) 4분반 소프트웨어학과 32202970 윤예진
* 소켓프로그래밍 과제 3 : 계산기 프로그램 (서버)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BUF_SIZE 256


// 서버에 수식을 전송 후 결과값을 수신하여 출력.
// 수식의 모든 요소는 공백으로 구분
// +, -, /, * 연산자를 지원하며 음수를 제외한 정수, 실수 입력 가능
void main(void)
{
    int cs = -1;            // client socket
    int sa = -1;            // socket accepted
    double result = 0;
    char buf[BUF_SIZE];

    // 소켓 주소 설정 및 초기화
    struct sockaddr_in csa;
    memset(&csa, 0, sizeof(csa));
    csa.sin_family = AF_INET;
    csa.sin_addr.s_addr = htonl(INADDR_ANY);
    csa.sin_port = htons(12345);

    // 소켓 생성 후 바인딩
    cs = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect(cs, (struct sockaddr*) &csa, sizeof(csa))< 0)
    {
        printf("Failed to connect");
        close(cs); exit(1);
    }

    // 수식 입력 후 전송
    printf("Enter expression\n");
    printf("- 피연산자와 연산자 모두 공백으로 구분\n- +, -, /, * 연산자 지원\n- 음수 x, 실수 입력 가능\n");
    printf(">>> ");
    fgets(buf, 256, stdin);
    send(cs, buf, BUF_SIZE, 0);

    // 결과값(double) 수신 후 출력
    recv(cs, (double*)&result, sizeof(double), 0);
    puts(buf);
    printf("\n= %lf\n", result);
    close(cs);
}