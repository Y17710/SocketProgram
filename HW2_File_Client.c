/** 컴퓨터네트워크(SW) 4분반 소프트웨어학과 32202970 윤예진
* 소켓프로그래밍 과제 2 : 파일 읽기/쓰기 통신 (클라이언트)
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

void read_file(int cs, char* buf, char* fname);
void write_file(int cs, char* buf, char* fname);
void request_file(int cs, char* buf, char* fname);


// 클라이언트: 요청 종류 전송 --> 파일 이름 전송 --> 작업 처리
// 3가지 기능 지원 - 텍스트 파일 읽기, 텍스트 파일 쓰기, 파일 다운로드(텍스트, 바이너리)
void main(void)
{
    int cs = -1;                // client socket
    int flag = -1;              // 1 = read, 2 = write, 3 = request
    char buf[BUF_SIZE];         // 버퍼
    char fname[BUF_SIZE];       // 파일 이름

    // 소켓 주소 설정 및 초기화
    struct sockaddr_in csa;
    memset(&csa, 0, sizeof(csa));
    csa.sin_family = AF_INET;
    csa.sin_addr.s_addr = htonl(INADDR_ANY);
    csa.sin_port = htons(13213);
    
    // 소켓 바인딩
    cs = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect(cs, (struct sockaddr*) &csa, sizeof(csa))< 0) {
        printf("Failed to connect");
        close(cs); exit(1);
    }

    // 서버에 요청할 작업의 종류와 파일 이름 입력
    printf("Read Text file(1) / Write Text file(2) / Request file(3) :");
    scanf("%d", &flag);
    if(flag == 1 || flag == 2 || flag == 3) {
        printf("Enter file name: ");
        scanf("%s", fname);
        getchar();
    }
    else {
        printf("Wrong option\n"); close(cs);
    }

    // 서버에 요청 종류 알리기(전송)
    *((int*)buf) = flag;
    send(cs, buf, BUF_SIZE, 0);

    if(flag==1)
        read_file(cs, buf, fname);
    else if(flag==2)
        write_file(cs, buf, fname);
    else if(flag==3)
        request_file(cs, buf, fname);

    close(cs);
}


// 서버로부터 파일을 다운로드하는 함수. 텍스트, 바이너리 파일 모두 지원.
void request_file(int cs, char* buf, char* fname)
{
    int fp = -1;                // file descriptor
    int fsize = 0;              // 파일 크기
    int remainsize = 0;         // 남은 받아야할 파일 크기

    send(cs, fname, BUF_SIZE, 0);                                  // 파일 이름 전송
    if(recv(cs, (char*)&fsize, sizeof(int), 0) < 0) return;        // 파일 사이즈 수신

    // 파일이 존재하지 않을 경우 종료
    if(fsize == 0) {
        printf("No file exist\n");
        close(cs); exit(0);
    }

    printf("Start downloading ... \n");
    fp = open(fname, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    remainsize = fsize;

    // BUF_SIZE(256) 단위로 파일 다운로드
    while(remainsize > BUF_SIZE) {
        recv(cs, buf, BUF_SIZE, 0);
        write(fp, buf, BUF_SIZE);
        remainsize -= BUF_SIZE;
    }
    // 남은 파일 다운로드
    if(remainsize>0) {
        recv(cs, buf, remainsize, 0);
        write(fp, buf, remainsize);
    }

    printf("Success download, File size: %d\n", fsize);
    close(fp);
}


// 서버에 저장된 텍스트 파일을 읽고 화면에 출력하는 함수.
void read_file(int cs, char* buf, char* fname)
{
    int fsize = 0;              // 파일 크기
    int remainsize = 0;         // 남은 받아야할 파일 크기

    send(cs, fname, BUF_SIZE, 0);                                  // 파일 이름 전송
    if(recv(cs, (char*)&fsize, sizeof(int), 0) < 0)                // 파일 사이즈 수신

    // 파일이 존재하지 않을 경우 종료
    if(fsize == 0) {
        printf("No file exist\n");
        close(cs); exit(0);
    }

    // 파일 크기 안내받음
    remainsize = fsize;
    printf("start to read: %s\n\n", fname);

    // BUF_SIZE(256) 단위로 화면에 출력
    while(remainsize > BUF_SIZE) {
        recv(cs, buf, BUF_SIZE, 0);
        puts(buf);
        remainsize -= BUF_SIZE;
    }
    // 버퍼에 남은 것을 화면에 출력
    if(remainsize>0) {
        recv(cs, buf, remainsize, 0);
        printf("%.*s\n", remainsize, buf);
    }

    printf("\nSuccess read, File size: %d\n", fsize);
    return;
}


// 서버에 텍스트 파일을 쓰는 함수.
// 서버는 이미 파일이 존재할 경우 파일끝에서부터 이어쓰기, 없을 경우 새로 파일 생성.
void write_file(int cs, char* buf, char* fname)
{
    int fsize = -1;

    send(cs, fname, BUF_SIZE, 0);                                  // 파일 이름 전송
    if(recv(cs, (char*)&fsize, sizeof(int), 0) < 0) return;        // 파일 사이즈 수신

    // 파일에 쓸 텍스트 입력
    printf("Enter text to write (Max 255byte):\n");
    fgets(buf, 256, stdin);

    // 서버에 텍스트 전송
    send(cs, buf, BUF_SIZE, 0);
    return;
}