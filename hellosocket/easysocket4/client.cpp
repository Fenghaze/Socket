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
        char sendbuf[BUFSIZ];
        // 从终端输入请求命令
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 处理请求
        if (strcmp(sendbuf, "exit\n") == 0)
            break;
        else if(strcmp(sendbuf, "login\n") == 0)
        {
            Login login = {"小红", "12345"};
            DataHeader hd = {sizeof(Login), CMD_LOGIN};
            // 先发包头
            send(cfd, &hd, sizeof(DataHeader), 0);
            // 再发包体
            send(cfd, &login, sizeof(Login), 0);
            // 接收服务器返回的数据
            recv(cfd, )
        }
        else if(strcmp(sendbuf, "logout\n") == 0)
        {
            Logout logout = {"小红"}
        }
        else
            printf("type error!\n");
        
    }

    close(cfd);
    return 0;
}