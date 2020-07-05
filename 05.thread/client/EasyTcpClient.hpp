#ifndef EASYTCPCLIENT_H__
#define EASYTCPCLIENT_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../server/proto.h"
#include <unistd.h>
#include <string.h>

#define INVALID_SOCKET -1
class EasyTcpClient
{
private:
    int cfd;
    // 第二缓冲区
    char szRecv[10240];
    char msgBuf[10240 * 10];
    int lastPos = 0;

public:
    EasyTcpClient()
    {
        cfd = INVALID_SOCKET;
    }

    virtual ~EasyTcpClient()
    {
        if (cfd != INVALID_SOCKET)
        {
            closeSocket();
            cfd = INVALID_SOCKET;
        }
    }

    // 创建socket
    int initSocket()
    {
        if (cfd != INVALID_SOCKET)
        {
            printf("close old connect...\n");
            closeSocket();
        }
        cfd = socket(AF_INET, SOCK_STREAM, 0);
        if (cfd < 0)
        {
            perror("socket()");
            return -1;
        }
        return cfd;
    }
    int getSocket()
    {
        return cfd;
    }
    // 连接服务端
    int Connect(char *ip, short port)
    {
        struct sockaddr_in laddr;
        if (cfd == INVALID_SOCKET)
            initSocket();

        laddr.sin_family = AF_INET;
        laddr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &laddr.sin_addr);
        if (connect(cfd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
        {
            perror("connect()");
            return -1;
        }
        return 0;
    }

    // 关闭socket
    void closeSocket()
    {
        if (cfd != INVALID_SOCKET)
        {
            close(cfd);
            cfd = INVALID_SOCKET;
        }
    }
    // 监听socket
    bool OnRun()
    {
        if (isRun())
        {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(cfd, &readfds);
            timeval t = {1, 0};
            int n = select(cfd + 1, &readfds, NULL, NULL, &t);
            if (n < 0)
            {
                printf("【socket=%d】", cfd);
                perror("select()");
                return false; /* code */
            }
            if (FD_ISSET(cfd, &readfds))
            {
                if (recvData(cfd) < 0)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    // 接收数据
    int recvData(int cfd)
    {
        // 接收服务端发送的数据
        int len = recv(cfd, &szRecv, 10240, 0);
        if (len <= 0)
        {
            printf("connect error!\n");
            return -1;
        }
        memcpy(msgBuf + lastPos, szRecv, len);
        // 更新msgBuf的数据长度
        lastPos += len;
        // 判断缓冲区的数据是否大于消息头
        while (lastPos >= sizeof(DataHeader))
        {
            DataHeader *header = (DataHeader *)msgBuf;
            if (lastPos >= header->dataLength)
            {
                lastPos -= header->dataLength;
                OnNetMsg(header);
                // 处理完一条消息后，将缓冲区的数据前移
                memcpy(msgBuf, msgBuf + header->dataLength, lastPos);
            }
            else
            {
                break;
            }
        }
        return 0;
    }

    // 处理接收到的数据
    void OnNetMsg(DataHeader *header)
    {
        // 处理请求
        switch (header->cmd)
        {
        case CMD_LOGIN_RES:
        {
            // 接收客户端发送的登录信息
            LoginResult *ret = (LoginResult *)header;
    //        printf("数据包长度：%d\t收到命令：%d\t", ret->dataLength, ret->cmd);
        //    printf("登录状态：%d\n", ret->result);
            break;
        }
        case CMD_LOGOUT_RES:
        {
            LogoutResult *ret = (LogoutResult *)header;
       //     printf("数据包长度：%d\t收到命令：%d\t", ret->dataLength, ret->cmd);
        //    printf("登出状态：%d\n", ret->result);
            break;
        }
        case CMD_NEW_USER_JOIN:
        {
            NewUserJoin *new_user = (NewUserJoin *)header;
         //   printf("数据包长度：%d\t收到命令：%d\t", new_user->dataLength, new_user->cmd);
         //   printf("new client join【%d】\n", new_user->sock);
            break;
        }
        case CMD_ERROR:
        {
            printf("收到错误消息，数据包长度：%d\n", header->dataLength);
            break;
        }
        default:
        {
            printf("收到未知消息，数据包长度：%d\n", header->dataLength);
            break;
        }
            
        }
    }

    // 发送数据
    int sendData(DataHeader *header)
    {
        if (isRun() && header)
        {
            send(cfd, (char *)header, header->dataLength, 0);
            return 0;
        }
        return -1;
    }

    // 判断socket是否在工作
    bool isRun()
    {
        return cfd != INVALID_SOCKET;
    }
};

#endif