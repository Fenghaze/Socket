/*
使用线程执行：输入命令来发送请求给服务器
使用select模型接收服务端的数据
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
#include <thread>
using namespace std;
static int client_work(int cfd)
{
    // 接收服务端发送的数据
    DataHeader header = {};
    int len = recv(cfd, &header, sizeof(DataHeader), 0);
    if (len <= 0)
    {
        printf("connect error!\n");
        return -1;
    }
    // 处理请求
    switch (header.cmd)
    {
    case CMD_LOGIN_RES:
    {
        // 接收客户端发送的登录信息
        LoginResult ret;
        recv(cfd, (char *)&ret + sizeof(DataHeader), sizeof(LoginResult) - sizeof(DataHeader), 0);
        printf("数据包长度：%d\t收到命令：%d\n", ret.dataLength, ret.cmd);
        printf("登录状态：%d\n", ret.result);
        break;
    }
    case CMD_LOGOUT_RES:
    {
        LogoutResult ret;
        recv(cfd, (char *)&ret + sizeof(DataHeader), sizeof(LogoutResult) - sizeof(DataHeader), 0);
        printf("数据包长度：%d\t收到命令：%d\n", ret.dataLength, ret.cmd);
        printf("登出状态：%d\n", ret.result);
        break;
    }
    case CMD_NEW_USER_JOIN:
    {
        NewUserJoin new_user;
        recv(cfd, (char *)&new_user + sizeof(DataHeader), sizeof(NewUserJoin) - sizeof(DataHeader), 0);
        printf("数据包长度：%d\t收到命令：%d\n", new_user.dataLength, new_user.cmd);
        printf("new client join【%d】\n", new_user.sock);
        break;
    }
    default:
        break;
    }
    return 0;
}

bool flag = true;

static void cmdThread(int cfd)
{
    while (1)
    {
        char sendbuf[BUFSIZ];
        // 从终端输入请求命令
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 处理请求
        if (strcmp(sendbuf, "exit\n") == 0)
        {
            flag = false;
            break;
        }
        else if (strcmp(sendbuf, "login\n") == 0)
        {
            // 向服务端发送登录信息
            Login login;
            strcpy(login.UserName, "xiaohong");
            strcpy(login.PassWord, "ferwr");
            send(cfd, (char *)&login, sizeof(Login), 0);
        }
        else if (strcmp(sendbuf, "logout\n") == 0)
        {
            Logout logout;
            strcpy(logout.UserName, "xiaohong");
            write(cfd, (char *)&logout, sizeof(logout));
        }
        else
            printf("type error!\n");
    }
}

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
    // 启动输入命令的线程
    thread t1(cmdThread, cfd);

    fd_set readfds;
    FD_ZERO(&readfds);
    while (flag)
    {
        FD_SET(cfd, &readfds);
        timeval t = {2, 0};
        int n = select(cfd + 1, &readfds, NULL, NULL, &t);
        if (n < 0)
        {
            perror("select()");
            break; /* code */
        }
        if (FD_ISSET(cfd, &readfds))
        {
            client_work(cfd);
        }
    }

    close(cfd);
    return 0;
}
