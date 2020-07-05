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
#include <thread>
#include <mutex>
#include "../../04.timer/CELLTimestamp.h"
#include <atomic>
#define INVALID_SOCKET -1
#define IPSIZE 40
#define SERVER_TNUM 1 //服务线程个数
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

class CellServer
{
private:
    int lfd;
    std::vector<ClientSocket *> g_clients; // 客户端cfd数组
    char szRecv[10240];
    std::thread *server_thread;
    std::vector<ClientSocket *> g_clientsBuf; // 缓冲队列，从这里面获取cfd
    std::mutex mut;

public:
    // 添加计数器
    std::atomic<int> recvCount;

    CellServer(int lfd)
    {
        this->lfd = lfd;
        server_thread = nullptr;
        recvCount = 0;
    }

    ~CellServer()
    {
    }
    // 判断socket是否在工作
    bool isRun()
    {
        return lfd != INVALID_SOCKET;
    }
    // 处理数据
    void OnNetMsg(DataHeader *header, int cfd)
    {
        recvCount++;
        switch (header->cmd)
        {
        case CMD_LOGIN:
        {
            Login *login = (Login *)header;
            // 接收客户端发送的登录信息
            //printf("-----------socket=%d----------\n", cfd);
            // printf("数据包长度：%d\t收到命令：%d\n", login->dataLength, login->cmd);
            //printf("username=%s\tpassword=%s\n", login->UserName, login->PassWord);
            // 返回登录状态
            // LoginResult ret;
            // sendData(&ret, cfd);
            break;
        }
        case CMD_LOGOUT:
        {
            Logout *logout = (Logout *)header;
            // 接收客户端发送的登出信息
            //   printf("-----------socket=%d----------\n", cfd);
            //  printf("数据包长度：%d\t收到命令：%d\n", logout->dataLength, logout->cmd);
            //   printf("username=%s\n", logout->UserName);
            // 返回登出状态
            //  LogoutResult ret;
            // sendData(&ret, cfd);
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
        return 0;
    }
    bool OnRun(int tid)
    {
        printf("线程【%d】启动\n", tid);
        while (isRun())
        {
            // 将缓冲队列的cfd加入到客户端cfd数组
            if (g_clientsBuf.size() > 0)
            {
                std::lock_guard<std::mutex> lg(mut);
                for (auto client : g_clientsBuf)
                {
                    g_clients.push_back(client);
                }
                g_clientsBuf.clear();
            }
            if (g_clients.empty())
            {
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
                continue;
            }
            fd_set readfds;
            FD_ZERO(&readfds);
            int maxfd = g_clients[0]->getSock();
            // 监听所有客户端cfd的读写事件
            for (int n = (int)g_clients.size() - 1; n >= 0; n--)
            {
                FD_SET(g_clients[n]->getSock(), &readfds);
                if (maxfd < g_clients[n]->getSock())
                {
                    maxfd = g_clients[n]->getSock();
                }
            }
            // 创建select,监听集合中的socket
            int ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
            if (ret < 0)
            {
                perror("select()");
                close(lfd);
                return -1;
            }
            // 处理每个客户端的请求
            for (int n = (int)g_clients.size() - 1; n >= 0; n--)
            {
                int cfd = g_clients[n]->getSock();
                if (FD_ISSET(cfd, &readfds))
                {
                    if (-1 == recvData(g_clients[n], cfd))
                    {
                        auto iter = g_clients.begin() + n;
                        if (iter != g_clients.end())
                        {
                            delete g_clients[n];
                            g_clients.erase(iter);
                        }
                    }
                }
            }
        }
    }

    // 缓冲队列一次只允许一个线程来操作
    void addClient(ClientSocket *client)
    {
        std::lock_guard<std::mutex> lg(mut);
        g_clientsBuf.push_back(client);
    }

    void StartThread(int tid)
    {
        server_thread = new std::thread(std::mem_fun(&CellServer::OnRun), this, tid);
    }

    // 获得数组中的cfd个数
    size_t getClientSize()
    {
        return g_clients.size() + g_clientsBuf.size();
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
    // 添加计时器
    CELLTimestamp timer;
    // 添加计数器
    int recvCount;
    std::vector<CellServer *> cell_servers;

public:
    EasyTcpServer()
    {
        lfd = INVALID_SOCKET;
        recvCount = 0;
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

    void addClient2Buf(ClientSocket *client)
    {
        g_clients.push_back(client);
        auto minServer = cell_servers[0];
        for (auto server : cell_servers)
        {
            if (minServer->getClientSize() > server->getClientSize())
            {
                minServer = server;
            }
        }
        minServer->addClient(client);
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

        // 打印客户端信息
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, sizeof(ipstr));
        printf("client[%s:%d]\n", ipstr, ntohs(raddr.sin_port));

        // 判断溢出
        if (g_clients.size() == FD_SETSIZE)
        {
            fputs("too many clients\n", stderr);
            return -1;
        }
        // 向缓冲区中加入新的客户端
        addClient2Buf(new ClientSocket(cfd));
        return 0;
    }

    // 启动服务线程
    void StartServers()
    {
        for (int i = 0; i < SERVER_TNUM; i++)
        {
            auto server = new CellServer(lfd);
            cell_servers.push_back(server);
            server->StartThread(i+1);
        }
    }

    // 监听是否有新的客户端请求连接
    bool OnRun()
    {
        if (isRun())
        {
            time4msg();
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            // 监听服务端的读事件，用于判断是否有新的客户端请求连接
            FD_SET(lfd, &readfds);
            timeval t = {1, 0};
            int n = select(lfd + 1, &readfds, NULL, NULL, &t);
            if (n < 0)
            {
                perror("select()");
                closeSocket();
                return false;
            }
            if (FD_ISSET(lfd, &readfds))
            {
                Accept();
                return true;
            }
        }
        return false;
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

    // 打印消息
    void time4msg()
    {
        auto t1 = timer.getElapsedSecond();
        if (t1 >= 1.0)
        {
            int allcount = 0;
            for (auto server : cell_servers)
            {
                allcount += server->recvCount;
                server->recvCount = 0;
            }
            printf("time【%lfs】,socket【%d】,当前客户端数量【%ld】,收到数据包个数【%d】\n", t1, lfd, g_clients.size(), allcount);
            timer.update();
        }
    }

    // 判断socket是否在工作
    bool isRun()
    {
        return lfd != INVALID_SOCKET;
    }
};
#endif