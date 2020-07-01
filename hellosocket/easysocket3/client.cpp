/*
输入命令来发送请求给服务器
接收结构化的数据
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "proto.h"
#include <unistd.h>
#include <string.h>

int main()
{
    int cfd;
    struct sockaddr_in laddr;
    char msgbuf[BUFSIZ];
    char sendbuf[BUFSIZ];
    struct DataPackage dp;
    // 1、建立socket
    cfd = socket(AF_INET, SOCK_STREAM, 0);

    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, "127.0.0.1", &laddr.sin_addr);

    // 2、向服务端套接字发送连接请求
    if (connect(cfd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
    {
        perror("connect()");
        exit(1);
    }

    while (1)
    {  
        // 从终端输入请求命令
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 处理请求
        if (strcmp(sendbuf, "exit\n") == 0)
            break;
        else
            // 向服务器发送请求命令
            write(cfd, sendbuf, strlen(sendbuf));
        // 接收服务端返回的数据并打印到终端
        recv(cfd, &dp, sizeof(dp), 0);
        printf("name=%s\n", dp.name);
        printf("age=%d\n", dp.age);
    }

    close(cfd);
    return 0;
}