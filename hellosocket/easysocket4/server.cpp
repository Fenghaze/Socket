/*
一次只接收一个客户端的请求，处理请求，发送数据给客户端
发送的数据是结构体类型
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
#include <string>
#define IPSIZE 40

using namespace std;

int main()
{
    int lfd, cfd;
    socklen_t raddr_len;
    struct sockaddr_in laddr, raddr;
    char ipstr[IPSIZE];
    // 1、创建socket
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

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
    cfd = accept(lfd, (struct sockaddr *)&raddr, &raddr_len);
    if (cfd < 0)
    {
        perror("accept()");
        exit(1);
    }
    inet_ntop(AF_INET, &raddr.sin_addr, ipstr, sizeof(ipstr));
    printf("client[%s:%d]\n", ipstr, ntohs(raddr.sin_port));
    while (1)
    {
        // 5、接收客户端发送的登录数据
        DataHeader header = {};
        int len = recv(cfd, &header, sizeof(DataHeader), 0);
        printf("数据包长度：%d\n", header.dataLength);
        printf("收到命令：%s\n", header.cmd);
        // 6、处理请求
        switch (header.cmd)
        {
        case CMD_LOGIN:
        {
            Login login = {};
            recv(cfd, &login, sizeof(Login), 0);
            LoginResult ret = {0};
            send(cfd, &header, sizeof(DataHeader), 0);
            send(cfd, &ret, sizeof(LoginResult), 0);
            break;
        }
        case CMD_LOGOUT:
        {
            Logout logout = {};
            recv(cfd, &logout, sizeof(Logout), 0);
            LogoutResult ret = {1};
            send(cfd, &header, sizeof(DataHeader), 0);
            send(cfd, &ret, sizeof(LogoutResult), 0);
            break;
        }
        default:
        {
            header.cmd = CMD_ERROR;
            header.dataLength = 0;
            send(cfd, &header, sizeof(DataHeader), 0);
            break;
        }
        }
    }
    close(lfd);
    close(cfd);

    return 0;
}
