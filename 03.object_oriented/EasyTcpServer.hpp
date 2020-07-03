#ifndef EASYTCPSERVER_H__
#define EASYTCPSERVER_H__

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

#define INVALID_SOCKET -1
#define IPSIZE 40

class ClientSocket
{
private:
    int cfd;
    char msgBuf[10240 * 10];
    int lastPos;

public:
    ClientSocket(int cfd)
    {
        this->cfd = cfd;
        memset(msgBuf, 0, sizeof(msgBuf));
        lastPos = 0;
    }
    int getSock()
    {
        return cfd;
    }
    char *getmsgBuf()
    {
        return msgBuf;
    }
    int getlastPos()
    {
        return lastPos;
    }
    void setlastPos(int lastPos)
    {
        this->lastPos = lastPos;
    }
};

class EasyTcpServer
{
private:
    int lfd;
    struct sockaddr_in laddr, raddr;
    fd_set readfds, writefds;
    socklen_t raddr_len;
    std::vector<ClientSocket *> g_clients;
    // 第二缓冲区
    char szRecv[10240];

public:
    EasyTcpServer()
    {
        lfd = INVALID_SOCKET;
    }
    ~EasyTcpServer()
    {
        if (lfd != INVALID_SOCKET)
        {
            closeSocket();
            lfd = INVALID_SOCKET;
        }
    }
    // 创建socket
    int initSocket()
    {
        if (lfd != INVALID_SOCKET)
        {
            printf("close old connect...\n");
            closeSocket();
        }
        lfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lfd < 0)
        {
            perror("socket()");
            return -1;
        }
        int val = 1;
        setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        return 0;
    }

    // 关闭socket
    void closeSocket()
    {
        if (lfd != INVALID_SOCKET)
        {
            close(lfd);
            lfd = INVALID_SOCKET;
        }
    }

    // 绑定ip、port
    int Bind(const char *ip, unsigned short port)
    {
        if (lfd == INVALID_SOCKET)
            initSocket();
        laddr.sin_family = AF_INET;
        laddr.sin_port = htons(port);
        if (!ip)
            ip = "0.0.0.0";
        inet_pton(AF_INET, ip, &laddr.sin_addr);
        if (bind(lfd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
        {
            perror("bind()");
            return -1;
        }
        return 0;
    }
    // 监听
    int Listen(int num)
    {
        if (listen(lfd, num) < 0)
        {
            perror("listen()");
            return -1;
        }
        return 0;
    }

    // 接受新客户端连接请求，并将该客户端加入动态数组
    int Accept()
    {
        int cfd;
        raddr_len = sizeof(raddr);
        cfd = accept(lfd, (struct sockaddr *)&raddr, &raddr_len);
        if (cfd < 0)
        {
            perror("accept()");
            return -1;
        }
        NewUserJoin new_user;
        new_user.sock = cfd;
        char ipstr[IPSIZE];

        // 向所有连接的客户端发送新的客户端消息
        sendData2All(&new_user);

        // 打印客户端信息
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, sizeof(ipstr));
        printf("client[%s:%d]\n", ipstr, ntohs(raddr.sin_port));

        // 判断溢出
        if (g_clients.size() == FD_SETSIZE)
        {
            fputs("too many clients\n", stderr);
            return -1;
        }
        // 向动态数组中加入新的客户端
        g_clients.push_back(new ClientSocket(cfd));
        return 0;
    }

    // 监听socket
    bool OnRun()
    {
        if (isRun())
        {
            int flag = true;
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            // 监听服务端的读事件，用于判断是否有新的客户端请求连接
            FD_SET(lfd, &readfds);
            int maxfd = -1;
            // 监听所有socket的读写事件
            for (size_t i = 0; i < g_clients.size(); i++)
            {
                FD_SET(g_clients[i]->getSock(), &readfds);
            }

            // 创建select,监听集合中的socket
            if (g_clients.size())
                maxfd = g_clients.back()->getSock(); // 最大文件描述符
            else
                maxfd = lfd;
            timeval t = {1, 0};
            int n = select(maxfd + 1, &readfds, NULL, NULL, &t);
            if (n < 0)
            {
                perror("select()");
                closeSocket();
                return -1;
            }
            // 当有新客户端要加入时，监听到服务端socket可读，此时接受客户端连接
            if (FD_ISSET(lfd, &readfds))
            {
                Accept();
                if (--n == 0) // 处理掉一个监听事件
                    flag = false;
            }

            // 处理每个客户端的请求
            if (flag)
            {
                int i = 0;
                auto it = g_clients.begin();
                while (it < g_clients.end())
                {
                    int cfd = (**it).getSock();
                    // 如果监听到事件，处理请求
                    if (FD_ISSET(cfd, &readfds))
                    {
                        // 客户端退出，删除这个客户端，不再监听
                        if (recvData(*it, cfd) < 0)
                        {
                            close(cfd);
                            // 释放当前客户端开辟的空间
                            delete g_clients[distance(g_clients.begin(), it)];
                            FD_CLR(cfd, &readfds);
                            it = g_clients.erase(it);
                        }
                        else
                            it++;
                        if (--n == 0)
                            break;
                    }
                    it++;
                }
            }
            printf("Spare time: Server working function...\n");
        }
    }

    // 发送数据给指定客户端
    int sendData(DataHeader *header, int cfd)
    {
        if (isRun() && header)
        {
            send(cfd, (char *)header, header->dataLength, 0);
            return 0;
        }
        return -1;
    }

    // 发送数据给所有客户端
    void sendData2All(DataHeader *header)
    {
        for (size_t i = 1; i < g_clients.size(); i++)
        {
            sendData(header, g_clients[i]->getSock());
        }
    }

    // 接收数据,处理粘包、少包问题
    int recvData(ClientSocket *client, int cfd)
    {
        // 接收客户端发送的登录数据
        int len = recv(cfd, &szRecv, 10240, 0);
        if (len <= 0)
        {
            printf("client【%d】 quit\n", cfd);
            return -1;
        }
        memcpy(client->getmsgBuf() + client->getlastPos(), szRecv, len);
        // 更新msgBuf的数据长度
        client->setlastPos(client->getlastPos() + len);
        // 判断缓冲区的数据是否大于消息头
        while (client->getlastPos() >= sizeof(DataHeader))
        {
            DataHeader *header = (DataHeader *)client->getmsgBuf();
            if (client->getlastPos() >= header->dataLength)
            {
                int cur_len = client->getlastPos() - header->dataLength;
                OnNetMsg(header, cfd);
                // 处理完一条消息后，将缓冲区的数据前移
                memcpy(client->getmsgBuf(), client->getmsgBuf() + header->dataLength, cur_len);
                client->setlastPos(cur_len);
            }
            else
            {
                break;
            }
        }
        LoginResult ret;
        sendData(&ret, cfd);
        return 0;
    }

    // 处理数据
    void OnNetMsg(DataHeader *header, int cfd)
    {
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            Login *login = (Login *)header;
            // 接收客户端发送的登录信息
            printf("-----------socket=%d----------\n", cfd);
            printf("数据包长度：%d\t收到命令：%d\n", login->dataLength, login->cmd);
            printf("username=%s\tpassword=%s\n", login->UserName, login->PassWord);
            // 返回登录状态
            LoginResult ret;
            sendData(&ret, cfd);
            break;
        }
        case CMD_LOGOUT:
        {
            Logout *logout = (Logout *)header;
            // 接收客户端发送的登出信息
            printf("-----------socket=%d----------\n", cfd);
            printf("数据包长度：%d\t收到命令：%d\n", logout->dataLength, logout->cmd);
            printf("username=%s\n", logout->UserName);
            // 返回登出状态
            LogoutResult ret;
            sendData(&ret, cfd);
            break;
        }
        default:
        {
            printf("收到未知消息，数据包长度：%d\n", header->dataLength);
            // DataHeader ret;
            //sendData(&ret, cfd);
            break;
        }
        }
    }

    // 判断socket是否在工作
    bool isRun()
    {
        return lfd != INVALID_SOCKET;
    }
};
#endif