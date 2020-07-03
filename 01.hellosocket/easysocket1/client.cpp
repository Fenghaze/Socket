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

    // 3、连接成功后，接收数据
    int len = recv(cfd, msgbuf, BUFSIZ, 0);
    write(1, msgbuf, len);
    
    close(cfd);
    return 0;
}