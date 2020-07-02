/*
使用select模型
一次接收多个客户端的请求，处理请求，发送数据给客户端
返回客户端的登录/登出状态
每当有新的客户端加入时，向所有连接的客户端发送新的客户端消息
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
#include <vector>
#define IPSIZE 40

using namespace std;

vector<int> g_clients;

static int server_work(int cfd)
{
    // 接收客户端发送的登录数据
    DataHeader header = {};
    int len = recv(cfd, &header, sizeof(DataHeader), 0);
    if (len <= 0)
    {
        printf("client quit\n");
        return -1;
    }
    // 处理请求
    switch (header.cmd)
    {
    case CMD_LOGIN:
    {
        Login login = {};
        // 接收客户端发送的登录信息
        recv(cfd, (char *)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
        printf("-----------socket=%d----------\n", cfd);
        printf("数据包长度：%d\t收到命令：%d\n", login.dataLength, login.cmd);
        printf("username=%s\tpassword=%s\n", login.UserName, login.PassWord);
        // 返回登录状态
        LoginResult ret;
        send(cfd, (char *)&ret, sizeof(LoginResult), 0);
        break;
    }
    case CMD_LOGOUT:
    {
        Logout logout = {};
        // 接收客户端发送的登出信息
        recv(cfd, (char *)&logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
        printf("-----------socket=%d----------\n", cfd);
        printf("数据包长度：%d\t收到命令：%d\n", logout.dataLength, logout.cmd);
        printf("username=%s\n", logout.UserName);
        // 返回登出状态
        LogoutResult ret;
        send(cfd, (char *)&ret, sizeof(LogoutResult), 0);
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
    return 0;
}

int main()
{
    int lfd, cfd;
    socklen_t raddr_len;
    struct sockaddr_in laddr, raddr;
    char ipstr[IPSIZE];
    fd_set readfds, writefds;
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
    // 重置集合
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    // 监听服务端的读事件，用于判断是否有新的客户端请求连接
    g_clients.push_back(lfd);
    int maxfd = -1;
    while (1)
    {
        // 监听所有socket的读写事件
        for (size_t i = 0; i < g_clients.size(); i++)
        {
            FD_SET(g_clients[i], &readfds);
        }

        // 创建select,监听集合中的socket
        maxfd = g_clients.back(); // 最大文件描述符
        timeval t = {1, 0};
        int n = select(maxfd + 1, &readfds, NULL, NULL, &t);
        if (n < 0)
        {
            perror("select()");
            exit(1);
        }
        // 当有新客户端要加入时，监听到服务端socket可读，此时接受客户端连接
        if (FD_ISSET(lfd, &readfds))
        {
            // 接受连接请求，获得一个可以向客户端传输数据的socket
            raddr_len = sizeof(raddr);
            cfd = accept(lfd, (struct sockaddr *)&raddr, &raddr_len);
            if (cfd < 0)
            {
                perror("accept()");
                exit(1);
            }
            NewUserJoin new_user;
            new_user.sock = cfd;
            // 向所有连接的客户端发送新的客户端消息
            for (size_t i = 1; i < g_clients.size(); i++)
                write(g_clients[i], (char *)&new_user, sizeof(NewUserJoin));
        
            inet_ntop(AF_INET, &raddr.sin_addr, ipstr, sizeof(ipstr));
            printf("client[%s:%d]\n", ipstr, ntohs(raddr.sin_port));
            // 判断溢出
            if (g_clients.size() - 1 == FD_SETSIZE)
            {
                fputs("too many clients\n", stderr);
                exit(1);
            }
            // 向动态数组中加入新的客户端
            g_clients.push_back(cfd);

            if (--n == 0) // 处理掉一个监听事件
                continue;
        }
        // 处理每个客户端的请求
        auto it = g_clients.begin() + 1;
        while (it < g_clients.end())
        {
            // 如果监听到事件，处理请求
            if (FD_ISSET(*it, &readfds))
            {
                // 客户端退出，删除这个客户端，不再监听
                if (server_work(*it) < 0)
                {
                    close(*it);
                    FD_CLR(*it, &readfds);
                    it = g_clients.erase(it);
                }
                else
                    it++;
                if (--n == 0)
                    break;
            }
            it++;
        }
        printf("Spare time: Server working function...\n");
    }
    close(lfd);
    return 0;
}
