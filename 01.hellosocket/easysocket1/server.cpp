#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "proto.h"
#include <unistd.h>
#include <string.h>
#define IPSIZE 40

int main()
{
    int lfd, cfd;
    socklen_t raddr_len;
    struct sockaddr_in laddr, raddr;
    char ipstr[IPSIZE];
    char msgbuf[] = "hello world!\n";
    // 1、创建socket
    lfd = socket(AF_INET, SOCK_STREAM, 0);

    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
    // 2、绑定端口和IP
    if (bind(lfd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
    {
        perror("bind()");
        exit(1);
    }

    // 3、监听socket
    if (listen(lfd, 128) < 0)
    {
        perror("listen()");
        exit(1);
    }

    raddr_len = sizeof(raddr);
    // 4、接受客户端发起的连接请求
    // 接受连接请求，获得一个可以向客户端传输数据的socket
    while (1)
    {
        cfd = accept(lfd, (struct sockaddr *)&raddr, &raddr_len);
        if (cfd < 0)
        {
            perror("accept()");
            exit(1);
        }
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, sizeof(ipstr));
        printf("client[%s:%d]\n", ipstr, ntohs(raddr.sin_port));

        // 5、利用客户端的socket，向客户端发送数据
        send(cfd, msgbuf, strlen(msgbuf), 0);
    }
    close(lfd);
    close(cfd);

    return 0;
}
