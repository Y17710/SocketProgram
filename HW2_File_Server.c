/** 컴퓨터네트워크(SW) 4분반 소프트웨어학과 32202970 윤예진
* 소켓프로그래밍 과제 2 : 파일 읽기/쓰기 통신 (서버)
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

void send_file(int sa, int ss, char* buf);
void write_file(int sa, int ss, char* buf);


// 서버: 요청 종류 수신 --> 파일 이름 수신 --> 요청 처리
void main(void)
{
    int flag = -1;          // 1 = read, 2 = write, 3 = request
    int ss = -1;            // server socket
    int sa = -1;            // saccepted socket
    char buf[BUF_SIZE];

    // 소켓 주소 설정 및 초기화
    struct sockaddr_in ssa;
    memset(&ssa, 0, sizeof(ssa));
    ssa.sin_family = AF_INET;
    ssa.sin_addr.s_addr = htonl(INADDR_ANY);
    ssa.sin_port = htons(13213);

    // 소켓 바인딩
    ss = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(bind(ss, (struct sockaddr*) &ssa, sizeof(ssa))<0)
    {
        printf("failed to bind\n");
        close(ss); exit(1);
    }

    listen(ss, 10);
    sa = accept(ss, 0, 0);

    // 사용자 요청 종류 수신 (1 = read, 2 = write, 3 = request)
    recv(sa, buf, BUF_SIZE, 0);
    flag = *((int*)buf);
    if(flag!=1 && flag != 2 && flag != 3) {
        printf("Wrong option\n");
        close(ss);
    }

    // 파일 이름 수신 
    recv(sa, buf, BUF_SIZE, 0);

    // 요청 처리
    if(flag == 1 || flag == 3) send_file(sa, ss, buf);
    else if(flag == 2) write_file(sa, ss, buf);

    close(ss);
}

// 파일 읽기 / 요청 처리 함수
// 파일 이름으로 탐색 후 파일 전송. 파일 읽기, 파일 저장은 클라이언트 측에서 처리.
void send_file(int sa, int ss, char* buf)
{
    int fp = -1;
    int fsize = -1;
    int remainsize = 0;

    fp = open(buf, O_RDONLY);
    // 해당 파일 존재하지 않을 시
    if(fp < 0) {
        printf("No file exist\n");
        send(sa, (char* )&fsize, sizeof(int), 0);
        close(fp); return;
    }

    printf("Success to Find\n");
    // 전송할 파일 크기 알림
    fsize = lseek(fp, 0, SEEK_END);
    remainsize = fsize;
    lseek(fp, 0, SEEK_SET);
    send(sa, (char* )&fsize, sizeof(int), 0);

    // 파일 전송
    while(remainsize>BUF_SIZE) {
        read(fp, buf, BUF_SIZE);
        send(sa, buf, BUF_SIZE, 0);
        remainsize -= BUF_SIZE;
    }
    if(remainsize>0) {
        read(fp, buf, remainsize);
        send(sa, buf, remainsize, 0);
        printf("Success to send, File size = %d\n", fsize);
    }

    close(fp);
}


// 파일 쓰기 처리 함수
// 파일이 없을 시 새로 파일을 생성 후 쓰기, 있을 시 기존 파일의 뒤에 이어서 쓰기
void write_file(int sa, int ss, char* buf)
{
    int fp = -1;
    int fsize = -1;
    fp = open(buf, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);

    fsize = lseek(fp, 0, SEEK_END); lseek(fp, 0, SEEK_SET);

    // 새로운 파일에 쓸 경우
    if(fsize==0) printf("Create new file to write\n");
    // 기존 파일에 이어쓸 경우
    else printf("Success to find\n");

    // 파일 크기 알림
    send(sa, (char* )&fsize, sizeof(int), 0);

    // 작성할 내용 클라이언트로부터 수신 후 파일에 작성
    recv(sa, buf, BUF_SIZE, 0);
    write(fp, buf, strlen(buf));

    close(fp);
    return;
}